#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mraa.h>
#include <sys/socket.h>
#include "include/blast_data.h"
#include "include/initialize.h"

sensor *build_sensor(char *label, char *name, char *unit, sensor_function update, sensor_type type, unsigned int pin)
{
	sensor *new_sensor = malloc(sizeof(sensor));
	new_sensor->label = malloc(strlen(label) + 1);
	strcpy(new_sensor->label, label);

	new_sensor->name = malloc(strlen(name) + 1);
	strcpy(new_sensor->name, name);

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
		/* pin is used to populate mux selection field */
		new_sensor->mux = pin;
		break;
	case TEST:
		break;
	}

	return new_sensor;
}

void destroy_sensor(sensor *sensor)
{
	free(sensor->label);
	free(sensor->name);
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

	/* initialize sensor key */
	*sensor_key = malloc(sizeof(sensor *) * sensor_count);

	/* parse line of format "string|string|integer" */
	for (i = 0; i < sensor_count; ++i) {
		char *label = line;
		char *label_end;
		char *name;
		char *name_end;
		char *next_line;
		int pin;
		sensor temp_sensor;

		/* break off current line from config string */
		next_line = strchr(line, '\n');
		*next_line = '\0';

		/* isolate the sensor label and pin number */
		label_end = strchr(line, '|');
		*label_end = '\0';
		name = label_end + 1;
		name_end = strchr(name, '|');
		*name_end = '\0';

		pin = atoi(name_end + (sizeof(char) * 1));

		/* find sensor in table and build a new sensor interface */
		temp_sensor = search_sensor(label);
		(*sensor_key)[i] = build_sensor(label, name, temp_sensor.unit,
			temp_sensor.update, temp_sensor.type, pin);

		/* jump to next line */
		line = next_line + (sizeof(char) * 1);
	}

	return sensor_count;
}

data_msg build_msg(const char *label, const char *name, const char *unit, unsigned long long timestamp, float data)
{
	data_msg msg;
	msg.label = malloc(strlen(label) + 1);
	strcpy(msg.label, label);

	msg.name = malloc(strlen(name) + 1);
	strcpy(msg.name, name);

	msg.unit = malloc(strlen(unit) + 1);
	strcpy(msg.unit, unit);

	msg.timestamp = timestamp;
	msg.data = data;

	return msg;
}

void destroy_msg(data_msg msg)
{
	free(msg.label);
	free(msg.name);
	free(msg.unit);

	return;
}

char *stringify_msg(data_msg new_msg)
{
    /* i hope 256 bytes is enough */
    char *msg_string = malloc(256);
    snprintf(msg_string, 256, "%s|%s|%s|%llu|%f", new_msg.label, new_msg.name, new_msg.unit, new_msg.timestamp, new_msg.data);

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

int select_line(int mux)
{
	/* set outputs here */
	mraa_gpio_write(pin10, mux & 0b1);
	mraa_gpio_write(pin11, mux & 0b10);
	mraa_gpio_write(pin12, mux & 0b100);
	mraa_gpio_write(pin13, mux & 0b1000);

	return 1;
}
