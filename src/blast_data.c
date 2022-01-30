#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "include/blast_data.h"

sensor *build_sensor(char *label, char *unit, sensor_function update)
{
	sensor *new_sensor = malloc(sizeof(sensor));
	new_sensor->label = malloc(strlen(label));
	strcpy(new_sensor->label, label);
	new_sensor->unit = malloc(strlen(unit));
	strcpy(new_sensor->unit, unit);
	new_sensor->update = update;

	return new_sensor;
}

void destroy_sensor(sensor *sensor)
{
	free(sensor->label);
	free(sensor->unit);
	free(sensor);

	return;
}

data_msg build_msg(const char *label, const char *unit, unsigned long long timestamp, double data)
{
	data_msg msg;
	msg.label = malloc(strlen(label) + 1);
	strcpy(msg.label, label);
	msg.unit = malloc(strlen(unit) + 1);
	strcpy(msg.unit, unit);
	msg.timestamp = timestamp;
	msg.data = data;

	return msg;
}

void destroy_msg(data_msg msg)
{
	free(msg.label);
	free(msg.unit);

	return;
}

char *stringify_msg(data_msg new_msg)
{
        /* i hope 256 bytes is enough */
        char *msg_string = malloc(256);
        snprintf(msg_string, 256, "%s|%s|%llu|%lf", new_msg.label, new_msg.unit, new_msg.timestamp, new_msg.data);
        
        return msg_string;
}

int send_msg(int sock, data_msg msg)
{
        char *msg_string;

        msg_string = stringify_msg(msg);

        if (send(sock, msg_string, strlen(msg_string), 0) < 0) {
                free(msg_string);
                return -1;
        }

        free(msg_string);
        return 0;
}
