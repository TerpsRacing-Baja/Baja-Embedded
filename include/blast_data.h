#ifndef BLAST_DATA_H
#define BLAST_DATA_H

#include <time.h>
#include <string.h>

typedef int (*sensor_function)(double *);

/* sensor interface structure */
typedef struct {
	char *label;
	char *unit;
	sensor_function update;
} sensor;

/* data structure for client -> server comms */
typedef struct {
	char *label;
	char *unit;
	time_t timestamp;
	double data;
} data_msg;

/* load all information for a single data point into a message structure */
data_msg build_msg(const char *label, const char *unit,
		   time_t timestamp, double data);

/* free the char pointers in a given data message */
void destroy_msg(data_msg msg);

/* convert data_msg structure into ANSI string for transfer.
   takes a data_msg struct and returns a char pointer. */
char *stringify_msg(data_msg new_msg);

/* given a data_msg and a socket, send it to the server (: */
int send_msg(int sock, data_msg msg);

#endif
