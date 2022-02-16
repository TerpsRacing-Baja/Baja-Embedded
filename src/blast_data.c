#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "include/blast_data.h"
#include "include/initialize.h"

sensor *build_sensor(char *label, char *unit, sensor_function update, sensor_type type, unsigned int pin)
{
	sensor *new_sensor = malloc(sizeof(sensor));
	new_sensor->label = malloc(strlen(label));
	strcpy(new_sensor->label, label);
	new_sensor->unit = malloc(strlen(unit));
	strcpy(new_sensor->unit, unit);

	new_sensor->update = update;

	new_sensor->type = type;

	switch (type) {
	case AIO:
		init_aio((mraa_aio_context)new_sensor->context, pin);
		break;
	case GPIO:
		init_gpio((mraa_gpio_context)new_sensor->context, pin);
		break;
	case I2C:
		/* implement later */
	}

	return new_sensor;
}

void destroy_sensor(sensor *sensor)
{
	free(sensor->label);
	free(sensor->unit);

	switch (sensor->type) {
	case AIO:
		mraa_aio_close((mraa_aio_context)sensor->context);
		break;
	case GPIO:
		mraa_gpio_close((mraa_gpio_context)sensor->context);
		break;
	case I2C:
		/* implement later */
	}

	free(sensor);

	return;
}

sensor search_sensor(char *name)
{
	int i = 0;

	while (sensor_table[i].label != NULL) {
		if (strcmp(name, sensor_table[i].label) == 0) {
			return sensor_table[i];
		}

		++i;
	}
	return sensor_table[i];
}

/* figure out how to do this smartly. sscanf() will probably be easiest but
it won't be efficient. temporarily partition each line and use indices to
mark strings? */
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

	/* initialize sensor key */
	*sensor_key = malloc(sizeof(sensor *) * sensor_count);

	/* string | string | enum | integer */
	for (i = 0; i < sensor_count; ++i) {
		char *name = line;
		char *name_end;
		int pin;
		sensor temp_sensor;
		char *next_line = strchr(line, '\n');
		*next_line = '\0';

		name_end = strchr(line, '|');
		*name_end = '\0';
		pin = atoi(name_end + (sizeof(char) * 1));

		temp_sensor = search_sensor(name);

		(*sensor_key)[i] = build_sensor(name, temp_sensor.unit,
			temp_sensor.update, temp_sensor.type, pin);

		line = next_line + (sizeof(char) * 1);
	}

	return sensor_count;	
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
