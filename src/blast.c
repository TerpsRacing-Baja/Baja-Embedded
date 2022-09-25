#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/mount.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>
#include <mraa.h>

#include "include/blast_data.h"
#include "include/sensors.h"

/** TODO:
 * sort out CAN nonsense
 * find an i2c sensor or three to test
 * 
 */

mraa_i2c_context i2c;	// shared i2c bus context
int interrupt = 0;	// track signals for shutdown

void handle_signal(int sig_type)
{
	if (sig_type == SIGINT || sig_type == SIGTERM) {
		fprintf(stderr, "guru meditation: shutting down due to interrupt ... \n");
		interrupt = 1;
	}

	return;
}

int main(int argc, char **argv)
{
	int sock;			// socket for connection to server
        struct sockaddr_in server;	// server address
	int num;			// number of configured sensors
	sensor **sensor_key;		// array of sensor interfaces
	char *config;			// buffer holding config information
	struct timeval tp;		// timeval struct for holding time info
	int sd_exists;			// set to true if there is an sd card
	char filename[70] = "/mnt/";	// name of log file
	FILE *logfile;			// file pointer to logfile

	/* custom handlers for sigint and sigterm */
	struct sigaction handler;
	handler.sa_handler = handle_signal;
	if (sigfillset(&handler.sa_mask) < 0) {
		fprintf(stderr, "guru mediation: failed to set signal masks\n");
		exit(1);
	}

	handler.sa_flags = 0;
	if (sigaction(SIGINT, &handler, 0) < 0) {
		fprintf(stderr, "guru meditation: failed to set new handler for SIGINT\n");
		exit(1);
	}

	if (sigaction(SIGTERM, &handler, 0) < 0) {
		fprintf(stderr, "guru meditation: failed to set new handler for SIGTERM\n");
		exit(1);
	}

	/* create an i2c context for the edison - exposed on bus 6 */
	i2c = mraa_i2c_init(6);

	/* only run if compiling on edison platform (not testing on other hardware)*/
	#ifndef TESTING
		if (i2c == NULL) {
			fprintf(stderr, "guru meditation: failed to initialize i2c-6 bus\n");
			mraa_deinit();
			exit(-1);
		}
	#endif

	/* TODO: run SD card check here */
	/* do an access and mount */
	/* open file based on datetime */
	if (access("/dev/mmcblk1p1", W_OK) == 0) {
		sd_exists = 1;
		if (mount("/dev/mmcblk1p1", "/mnt", "vfat", MS_NOATIME, NULL) == 0) {
			gettimeofday(&tp, NULL);
			struct tm *tm = localtime(&tp);
			strftime(filename + 5, sizeof(filename) - 5, "%Y-%m-%d_%H-%M-%S", tm);

			logfile = fopen(filename, "ab+");
		} else {
			fprintf(1, "Failed to mount SD card\n");
			sd_exists = 0;
		}
	} else {
		sd_exists = 0;
	}

        /* socket creation and server addressing */
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                perror("guru meditation");
                exit(1);
        }

        memset(&server, 0, sizeof server);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("45.79.166.190");
        server.sin_port = htons(65432);

        /* establish connection to server */
        if (connect(sock, (struct sockaddr *)&server, sizeof server) < 0) {
                perror("guru meditation");
                close(sock);
		mraa_deinit();
                exit(1);
        }

	#ifdef DEBUG
		printf("[*] connected to blast server\n");
	#endif

	/* receive configuration data from server */
	config = malloc(5096);
	for (int recvd_msg_size = 0; recvd_msg_size < (5096 - 1); ++recvd_msg_size) {
		if (recv(sock, config + recvd_msg_size, 1, 0) < 0) {
			perror("guru meditation");
			close(sock);
			free(config);
			mraa_deinit();
			exit(1);
		}

                if (*(config + recvd_msg_size) == '&') {
                        *(config + recvd_msg_size) = '\0';
                        break;
                }
        }

	#ifdef DEBUG
		printf("[*] received config:\n%s\n", config);
	#endif

	/* perform dynamic sensor configuration */
	num = configure_sensors(config, &sensor_key);
	free(config);

	#ifdef DEBUG
		printf("[*] data collection loop:\n");
	#endif

	/* data collection loop */
	while (!interrupt) {
		for (int i = 0; i < num; ++i) {
			data_msg msg;
			float *update_data = malloc(sizeof(float));

			if (sensor_key[i]->update(update_data, sensor_key[i]->context) < 0) {
				printf("Sensor %s failed on update\n", sensor_key[i]->label);
			} else {
				gettimeofday(&tp, NULL);
				msg = build_msg(sensor_key[i]->label,
					sensor_key[i]->unit,
					((unsigned long long)tp.tv_sec * 1000) + (tp.tv_usec / 1000),
					*update_data);

				#ifdef DEBUG
					char *temp = stringify_msg(msg);
					printf("[*] %s\n", temp);
					free(temp);
				#endif

				/* TODO: save to SD here */
				/* do fwrite */
				if (sd_exists) {
					char *msg_string = stringify_msg(msg);

					fwrite(msg_string, strlen(msg_string), 1, logfile);
					fwrite("\n", 1, 1, logfile);

					free(msg_string);
				}

				if (send_msg(sock, msg) < 0) {
					perror("guru meditation");
					close(sock);
					destroy_msg(msg);
					free(update_data);

					/* destroy sensors */
					for (int i = 0; i < num; ++i) {
						destroy_sensor(sensor_key[i]);
					}

					free(sensor_key);
					mraa_i2c_stop(i2c);
					mraa_deinit();
					exit(1);
				}

				destroy_msg(msg);
			}

			free(update_data);
		}

		sleep(1);
	}

	/* if interrupted, clean everything up */
	close(sock);

	for (int i = 0; i < num; ++i) {
		destroy_sensor(sensor_key[i]);
	}

	free(sensor_key);
	mraa_i2c_stop(i2c);
	mraa_deinit();
	exit(1);
}
