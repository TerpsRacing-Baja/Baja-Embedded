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
 * receive return message from server upon connection (config)
 * 
 * connection failsafes (filtered by signal type)
 * implement mraa lib
 * 	-> futz with pressure sensor
 * compiler debug flags
 * 
 * work w/ cap on why reconnect sometimes fails
 * 
 */

int main(int argc, char **argv)
{
	int sock;
        struct sockaddr_in server;
	int num;
	sensor **sensor_key;

	/* read pin config from file */
		// should account for multiplex channel - make helper functions
		// pin/datatype struct? could embed in datapoint struct

	/* mraa/pin configuration (for gpio and aio) */
	
	/* set test config for dummy sensor */
	sensor_key = malloc(sizeof(sensor) * 2);
	sensor_key[0] = build_sensor("Test Sensor", "Testies", test_sensor, TEST, 0);
	sensor_key[1] = build_sensor("FPS Pressure Sensor", "Pascals", fps_v2_range_5v, AIO, 0);
	num = 2;

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

	struct timeval tp;

	/* data collection loop */
	for (;;) {
		for (int i = 0; i < num; ++i) {
			data_msg test_msg;
			double *update_data = malloc(sizeof(double));

			if (sensor_key[i]->update(update_data) < 0) {
				printf("Sensor %s failed on update\n", sensor_key[i]->label);
			} else {
				gettimeofday(&tp, NULL);
				test_msg = build_msg(sensor_key[i]->label,
					sensor_key[i]->unit,
					(tp.tv_sec * 1000 + tp.tv_usec / 1000),
					*update_data);

				if (send_msg(sock, test_msg) < 0) {
					perror("guru meditation");
					close(sock);
					destroy_msg(test_msg);
					free(update_data);

					/* destroy sensors */
					for (int i = 0; i < num; ++i) {
						destroy_sensor(sensor_key[i]);
					}

					exit(1);
				}

				destroy_msg(test_msg);
				sleep(1);
			}

			free(update_data);
		}
	}
}
