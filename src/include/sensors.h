#ifndef SENSORS_H
#define SENSORs_H

#include "include/blast_data.h"

/* I2C device addresses */
#define MLX90614_ADDR 0x5A
#define MLX90614_TOB1 0x07

/* prototypes for test sensor */

/**
 * generates random data and assigns it to a double *
 * 
 */
int test_sensor(float *ptr, mraa_context context);

/**
 * reads and calibrates data from the FPS V2 Range 5V Pressure Sensor 
 * 
 */
int fps_v2_range_5v(float *ptr, mraa_context context);


/**
 * reads data from the mlx90614 i2c temperature sensor 
 * 
 */
int mlx90614(float *ptr);

#endif
