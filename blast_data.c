#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/blast_data.h"

data_msg build_msg(const char *label, const char *unit, time_t timestamp, double data)
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
        /* i hope 128 bytes is enough */
        char *msg_string = malloc(128);
        snprintf(msg_string, 128, "%s|%s|%i|%lf", new_msg.label, new_msg.unit, new_msg.timestamp, new_msg.data);
        
        return msg_string;
}
