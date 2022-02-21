#ifndef BLAST_DATA_H
#define BLAST_DATA_H

#include <time.h>
#include <string.h>
#include <mraa.h>

/**
 * any of the three relevant mraa contexts.
 * 
 */
typedef void * mraa_context;

/**
 * opaque definition of a pointer to a sensor update function.
 * 
 */
typedef int (*sensor_function)(double *, mraa_context);


/**
 * enumeration describing possible sensor types.
 * analog, digital i/o, i2c, and test types are available.
 * 
 */
typedef enum {
	AIO,
	GPIO,
	I2C,
	TEST
} sensor_type;

/**
 * opaque structure describing a sensor interface.
 * sensors have a label, a unit, an update function, a type, and a pin context.
 * 
 */
typedef struct {
	char *label;
	char *unit;
	sensor_function update;
	sensor_type type;
	mraa_context context;
} sensor;

/**
 * opaque structure describing a message sent to the blast server.
 * messages have a label, a unit, a timestamp, and some data.
 * 
 */
typedef struct {
	char *label;
	char *unit;
	unsigned long long timestamp;
	double data;
} data_msg;

/**
 * a table of hardcoded sensor configurations, used to match a label to its
 * units, type, and update function in dynamic configuration. 
 * 
 */
extern sensor sensor_table[];

/**
 * shared i2c context.
 * 
 */
extern mraa_i2c_context i2c;

/**
 * allocate memory for a sensor given its components.
 * 
 */
sensor *build_sensor(char *label, char *unit, sensor_function update, sensor_type type, unsigned int pin);

/**
 * deallocate a sensor and its components.
 * 
 */
void destroy_sensor(sensor *sensor);

/**
 * search for a label in the sensor table.
 * 
 */
sensor search_sensor(char *label);

/**
 * given a dynamic configuration string and the address of the sensor key,
 * build and configure all sensors on the car.
 * 
 */
int configure_sensors(char *config, sensor ***sensor_key);

/**
 *  load all information for a single data point into a message structure.
 * 
 */
data_msg build_msg(const char *label, const char *unit,
		   unsigned long long timestamp, double data);

/**
 *  free the char pointers in a given data message.
 * 
 */
void destroy_msg(data_msg msg);

/**
 * convert data_msg structure into ANSI string for transfer.
 * takes a data_msg struct and returns a char pointer.
 * 
 */
char *stringify_msg(data_msg new_msg);

/**
 * given a data_msg and a socket, send it to the server
 * 
 */
int send_msg(int sock, data_msg msg);

#endif
