#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "include/blast_data.h"
#include "include/initialize.h"

// added name char to reflect changes made build_sensor declaration in blast_data.h interface 
sensor *build_sensor(char *label, char *name, char *unit, sensor_function update, sensor_type type, unsigned int pin)
{
	sensor *new_sensor = malloc(sizeof(sensor));
	new_sensor->label = malloc(strlen(label) + 1);
	strcpy(new_sensor->label, label);

	// created space for value @ *name in new_sensor->name
	new_sensor->name = malloc(strlen(name)+1);
	strcopy(new_sensor->name, name);

	new_sensor->unit = malloc(strlen(unit) + 1);
	strcpy(new_sensor->unit, unit);

	new_sensor->update = update;

	new_sensor->type = type;

	switch (type) {
	case AIO:
		init_aio((mraa_aio_context *)&new_sensor->context, pin);
		break;
	case GPIO:
		init_gpio((mraa_gpio_context *)&new_sensor->context, pin);
		break;
	case I2C:
		init_i2c();
		break;
	case TEST:
		break;
	}

	return new_sensor;
}

void destroy_sensor(sensor *sensor)
{
	free(sensor->label);
	free(sensor->name); // frees mem referenced by sensor->name
	free(sensor->unit);

	switch (sensor->type) {
	case AIO:
		mraa_aio_close(*(mraa_aio_context *)sensor->context);
		break;
	case GPIO:
		mraa_gpio_close(*(mraa_gpio_context *)sensor->context);
		break;
	case I2C:
		break;
	case TEST:
		break;
	}

	free(sensor);

	return;
}

sensor search_sensor(char *label)
{
	int i = 0;

	while (sensor_table[i].label != NULL) {
		if (strcmp(label, sensor_table[i].label) == 0) {
			return sensor_table[i];
		}

		++i;
	}
	return sensor_table[i];
}

/* three star lol */
/* i know this could be done without three stars but it's funny */
int configure_sensors(char *config, sensor ***sensor_key)
{
	char *line = config;
	int i = 0;
	int sensor_count = 0;

	/* number of lines = number of sensors */
	while (*(line + (sizeof(char) * i)) != '\0') {
		if (*(line + (sizeof(char) * i)) == '\n') {
			++sensor_count;
		}

		++i;
	}

	/**
	 * 
	 * i need to query the racecapture here in order to find out
	 * how many sensors it hosts, and their information.
	 * 
	 * afterwards, i need to add that info to a separate data structure,
	 * and initialize the mcp2515 in the sensor key. note that there
	 * is some special behavior for the mcp2515 "sensor"!
	 * 
	 */

	/* initialize sensor key */
	*sensor_key = malloc(sizeof(sensor *) * sensor_count);

	/* parse line of format "string|integer" */
	for (i = 0; i < sensor_count; ++i) {
		char *label = line;
		char *label_end;
		char *next_line;
		int pin;
		sensor temp_sensor;
		
		/* break off current line from sconfig string */
		next_line = strchr(line, '\n');
		*next_line = '\0';

		/* isolate the sensor label and pin number */
		label_end = strchr(line, '|');
		*label_end = '\0';
		pin = atoi(label_end + (sizeof(char) * 1));

		/* find sensor in table and build a new sensor interface */
		temp_sensor = search_sensor(label);
		(*sensor_key)[i] = build_sensor(label, temp_sensor.unit,
			temp_sensor.update, temp_sensor.type, pin);

		/* jump to next line */
		line = next_line + (sizeof(char) * 1);
	}

	return sensor_count;	
}

// added name param to method signature to reflect changes to interface edits of build_msg
data_msg build_msg(const char *label, const char *name, const char *unit, unsigned long long timestamp, float data)
{
	data_msg msg;
	msg.label = malloc(strlen(label) + 1);
	strcpy(msg.label, label);

	// allcoates mem referenced by *name to msg.name
	msg.name = malloc(strlen(name) + 1);
	strcopy(msg.name, name);

	msg.unit = malloc(strlen(unit) + 1);
	strcpy(msg.unit, unit);

	msg.timestamp = timestamp;
	msg.data = data;

	return msg;
}

void destroy_msg(data_msg msg)
{
	free(msg.label);
	free(msg.name); // frees mem referenced by msg.name
	free(msg.unit);

	return;
}

char *stringify_msg(data_msg new_msg)
{
        /* i hope 256 bytes is enough */
        char *msg_string = malloc(256);
		// added new_msg.name as arg
        snprintf(msg_string, 256, "%s|%s|%llu|%f", new_msg.label, new_msg.name, new_msg.unit, new_msg.timestamp, new_msg.data);
        
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
