#ifndef SENSORS_H
#define SENSORs_H

#include "include/blast_data.h"

/* prototypes for test sensor */

/**
 * generates random data and assigns it to a double *
 * 
 */
int test_sensor(double *ptr, mraa_context context);

/**
 * reads and calibrates data from the FPS V2 Range 5V Pressure Sensor 
 * 
 */
int fps_v2_range_5v(double *ptr, mraa_context context);

#endif
