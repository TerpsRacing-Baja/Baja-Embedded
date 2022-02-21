#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <mraa.h>
#include "include/blast_data.h"
#include "include/sensors.h"

/** TODO:
 * calibrate and test pressure sensor
 * connection failsafes (filtered by signal type)
 * make code cleanup consistent / eradicate evil leaks
 * 
 */

mraa_i2c_context i2c;

int main(int argc, char **argv)
{
	int sock;			// socket for connection to server
        struct sockaddr_in server;	// server address
	int num;			// number of configured sensors
	sensor **sensor_key;		// array of sensor interfaces
	char *config;			// buffer holding config information
	struct timeval tp;		// timeval struct for holding time info

	/* initialize mraa library */
	mraa_init();

	/* create an i2c context for the edison - exposed on bus 6 */
	i2c = mraa_i2c_init(6);

	/* only run if compiling on edison platform (not testing on other hardware)*/
	#ifndef TESTING
		if (i2c == NULL) {
			mraa_deinit();
			fprintf(stderr, "guru meditation: failed to initialize i2c-6 bus\n");
			exit(-1);
		}
	#endif

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

	#ifdef DEBUG
		printf("[*] data collection loop:\n");
	#endif

	/* data collection loop */
	for (;;) {
		for (int i = 0; i < num; ++i) {
			data_msg msg;
			float *update_data = malloc(sizeof(float));

			if (sensor_key[i]->update(update_data, sensor_key[i]->context) < 0) {
				printf("Sensor %s failed on update\n", sensor_key[i]->label);
			} else {
				gettimeofday(&tp, NULL);
				msg = build_msg(sensor_key[i]->label,
					sensor_key[i]->unit,
					(tp.tv_sec * 1000 + tp.tv_usec / 1000),
					*update_data);

				#ifdef DEBUG
					printf("[*] %s\n", stringify_msg(msg));
				#endif

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
}
