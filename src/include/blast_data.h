#ifndef BLAST_DATA_H
#define BLAST_DATA_H

#include <time.h>
#include <string.h>
#include <mraa.h>

/**
 * any of the three relevant mraa contexts.
 * these are defined as void * by mraa anyway, so this is just
 * a typing convenience for passing contexts around
 *
 */
typedef void * mraa_context;

/**
 * opaque definition of a pointer to a sensor update function.
 *
 */
typedef int (*sensor_function)(float *, mraa_context);


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
 * sensors have a label, a name, a unit, an update function, a most recent
 * value, a type, and a pin context.
 *
 * additionally, if a sensor uses i2c, the mux field will be populated
 * with its multiplexer selection address.
 *
 */
typedef struct {
	char *label;
	char *name;
	char *unit;
	int mux;
	float val;
	sensor_function update;
	sensor_type type;
	mraa_context context;
} sensor;

/**
 * opaque structure describing a message sent to the blast server.
 * messages have a label, a unit, a timestamp, and some data.
 *
 * this structure also corresponds to the format of data saved
 * to log files
 *
 */
typedef struct {
	char *label;
	char *name;
	char *unit;
	unsigned long long timestamp;
	float data;
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
 * shared GPIO contexts
 *
 */
extern mraa_gpio_context pin10, pin11, pin12, pin13;

/**
 * allocate memory for a sensor given its components.
 *
 */
sensor *build_sensor(char *label, char *name, char *unit, sensor_function update, sensor_type type, unsigned int pin);

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
data_msg build_msg(const char *label, const char *name, const char *unit,
		   unsigned long long timestamp, float data);

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

/**
 * given a multiplexer selection, enable that line using digital outs.
 *
 * this function will use pins 10, 11, 12, and 13 on the Intel Edison
 *
 */
int select_line(int mux);

#endif
