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

mraa_i2c_context i2c;				// shared i2c bus context
mraa_gpio_context pin10, pin11, pin12, pin13;	// GPIO pins
int interrupt = 0;				// track signals for shutdown

void handle_signal(int sig_type)
{
	if (sig_type == SIGINT || sig_type == SIGTERM) {
		fprintf(stderr, "profound meditation: shutting down due to interrupt ... \n");
		interrupt = 1;
	}

	return;
}

int main(int argc, char **argv)
{
	int sock;			// socket for connection to server
	int sd_exists;			// set to true if there is an sd card
	int network_exists;		// set to true if there is network connectivity
	int num;			// number of configured sensors
        struct sockaddr_in server;	// server address
	struct timeval tp;		// timeval struct for holding time info
	char filename[70] = "/mnt/";	// name of log file
	char *config;			// buffer holding config information
	FILE *logfile;			// file pointer to logfile
	sensor **sensor_key;		// array of sensor interfaces

	/* custom handlers for sigint and sigterm */
	struct sigaction handler;
	handler.sa_handler = handle_signal;
	if (sigfillset(&handler.sa_mask) < 0) {
		fprintf(stderr, "profound mediation: failed to set signal masks\n");
		exit(1);
	}

	handler.sa_flags = 0;
	if (sigaction(SIGINT, &handler, 0) < 0) {
		fprintf(stderr, "profound meditation: failed to set new handler for SIGINT\n");
		exit(1);
	}

	if (sigaction(SIGTERM, &handler, 0) < 0) {
		fprintf(stderr, "profound meditation: failed to set new handler for SIGTERM\n");
		exit(1);
	}

	/* create an i2c context for the edison - exposed on bus 6 */
	i2c = mraa_i2c_init(6);

	/* only run if compiling on edison platform (not testing on other hardware)*/
	#ifndef TESTING
		if (i2c == NULL) {
			fprintf(stderr, "profound meditation: failed to initialize i2c-6 bus\n");
			mraa_deinit();
			exit(-1);
		}

		/* we will be using digital pins 10, 11, 12, and 13 */
		if (!(pin10 = mraa_gpio_init(10))) {
			fprintf(stderr, "Failed to initialize GPIO pin 10\n");
		}

		if (!(pin11 = mraa_gpio_init(11))) {
			fprintf(stderr, "Failed to initialize GPIO pin 11\n");
		}

		if (!(pin12 = mraa_gpio_init(12))) {
			fprintf(stderr, "Failed to initialize GPIO pin 12\n");
		}

		if (!(pin13 = mraa_gpio_init(13))) {
			fprintf(stderr, "Failed to initialize GPIO pin 13\n");
		}

		if (mraa_gpio_dir(pin10, MRAA_GPIO_OUT) != MRAA_SUCCESS) {
			fprintf(stderr, "Failed to set GPIO pin 10 to output mode\n");
		}

		if (mraa_gpio_dir(pin11, MRAA_GPIO_OUT) != MRAA_SUCCESS) {
			fprintf(stderr, "Failed to set GPIO pin 11 to output mode\n");
		}

		if (mraa_gpio_dir(pin12, MRAA_GPIO_OUT) != MRAA_SUCCESS) {
			fprintf(stderr, "Failed to set GPIO pin 12 to output mode\n");
		}

		if (mraa_gpio_dir(pin13, MRAA_GPIO_OUT) != MRAA_SUCCESS) {
			fprintf(stderr, "Failed to set GPIO pin 13 to output mode\n");
		}
	#endif

	/* do an access and mount sd if it exists */
	/* open file based on datetime */
	if (access("/dev/mmcblk1p1", W_OK) == 0) {
		sd_exists = 1;
		if (mount("/dev/mmcblk1p1", "/mnt", "vfat", MS_NOATIME, NULL) == 0) {
			time_t raw_time;
			time(&raw_time);
			struct tm *tm = localtime(&raw_time);
			strftime(filename + 5, sizeof(filename) - 5, "%Y-%m-%d_%H-%M-%S", tm);

			logfile = fopen(filename, "ab+");

			fwrite("Label|Name|Unit|Timestamp|Value\n", 32, 1, logfile);
		} else {
			fprintf(stderr, "Failed to mount SD card\n");
			sd_exists = 0;
		}
	} else {
		sd_exists = 0;
	}

        /* socket creation and server addressing */
        if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
                perror("profound meditation");
                exit(1);
        }

        memset(&server, 0, sizeof server);
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("45.79.166.190");
        server.sin_port = htons(65432);

        /* establish connection to server */
        if (connect(sock, (struct sockaddr *)&server, sizeof server) < 0) {
		network_exists = 0;
		perror("profound meditation");
        } else {
		network_exists = 1;
	}

	#ifdef DEBUG
		if (network_exists) {
			printf("[*] connected to blast server\n");
		}
	#endif

	/* receive configuration data from server */
	config = malloc(5096);
	if (network_exists) {
		for (int recvd_msg_size = 0; recvd_msg_size < (5096 - 1); ++recvd_msg_size) {
			if (recv(sock, config + recvd_msg_size, 1, 0) < 0) {
				perror("profound meditation");
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

		FILE *config_file = fopen("./configuration", "w");
		fwrite(config, 1, strlen(config)+1, config_file);
	}
	else {
		FILE *config_file = fopen("./configuration", "r");
		fread(config, 1, 5096, config_file);
	}
	
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
			float update_data;

			if (sensor_key[i]->type == I2C) {
				select_line(sensor_key[i]->mux);
			}

			if (sensor_key[i]->update(&update_data, sensor_key[i]->context) < 0) {
				printf("Sensor %s failed on update\n", sensor_key[i]->label);
			} else {
				gettimeofday(&tp, NULL);
				msg = build_msg(sensor_key[i]->label, sensor_key[i]->name,
					sensor_key[i]->unit,
					((unsigned long long)tp.tv_sec * 1000) + (tp.tv_usec / 1000),
					update_data);

				#ifdef DEBUG
					char *temp = stringify_msg(msg);
					printf("[*] %s\n", temp);
					free(temp);
				#endif

				if (sd_exists) {
					char *msg_string = stringify_msg(msg);

					fwrite(msg_string, strlen(msg_string), 1, logfile);
					fwrite("\n", 1, 1, logfile);
					fflush(logfile);

					free(msg_string);
				}

				if (network_exists && send_msg(sock, msg) < 0) {
					perror("profound meditation");
					close(sock);
					destroy_msg(msg);

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
	mraa_gpio_close(pin10);
	mraa_gpio_close(pin11);
	mraa_gpio_close(pin12);
	mraa_gpio_close(pin13);
	mraa_deinit();
	if (sd_exists) {
		fclose(logfile);
		umount("/mnt");
	}
	exit(1);
}
