#ifndef SENSORS_H
#define SENSORs_H

/* prototypes for test sensor */

/**
 * generates random data and assigns it to a double *
 */
int test_sensor(double *ptr);

/**
 * reads and calibrates data from the FPS V2 Range 5V Pressure Sensor 
 */
int fps_v2_range_5v(double *ptr);

#endif
