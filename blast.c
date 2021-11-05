#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
//#include <mraa.h>
#include "include/blast_data.h"

int main(int argc, char **argv)
{
	int sock;
        struct sockaddr_in server;
	data_msg test_msg;
	int num;
	sensor **sensor_key;

	/* read pin config from file */
		// should account for multiplex channel - make helper functions
		// pin/datatype struct? could embed in datapoint struct

	/* mraa/pin configuration (for gpio and aio) */

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

	/* data collection loop */
	for (;;) {

		for (int i = 0; i < num; ++i) {
			test_msg = build_msg(sensor_key[i]->label,
				sensor_key[i]->unit, time(NULL),
				sensor_key[i]->update());

			if (send_msg(sock, test_msg) < 0) {
				perror("guru meditation");
				close(sock);
				exit(1);
			}

			destroy_msg(test_msg);
		}
	}
}
