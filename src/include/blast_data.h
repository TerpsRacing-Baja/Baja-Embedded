#ifndef BLAST_DATA_H
#define BLAST_DATA_H

#include <time.h>
#include <string.h>
#include <mraa.h>

typedef int (*sensor_function)(double *);

typedef enum {
	AIO,
	GPIO,
	I2C,
	TEST
} sensor_type;

/* sensor interface structure */
typedef struct {
	char *label;
	char *unit;
	sensor_function update;
	sensor_type type;
	void *context;
} sensor;

/* data structure for client -> server comms */
typedef struct {
	char *label;
	char *unit;
	unsigned long long timestamp;
	double data;
} data_msg;

/* table of sensor configs */
extern sensor sensor_table[];

/* build a new sensor */
sensor *build_sensor(char *label, char *unit, sensor_function update, sensor_type type, unsigned int pin);

/* destroy a sensor */
void destroy_sensor(sensor *sensor);

/* search for a sensor in the sensor config table */
sensor search_sensor(char *name);

/* configure all sensors and return number of configured sensors */
int configure_sensors(char *config, sensor ***sensor_key);

/* load all information for a single data point into a message structure */
data_msg build_msg(const char *label, const char *unit,
		   unsigned long long timestamp, double data);

/* free the char pointers in a given data message */
void destroy_msg(data_msg msg);

/* convert data_msg structure into ANSI string for transfer.
   takes a data_msg struct and returns a char pointer. */
char *stringify_msg(data_msg new_msg);

/* given a data_msg and a socket, send it to the server (: */
int send_msg(int sock, data_msg msg);

#endif
