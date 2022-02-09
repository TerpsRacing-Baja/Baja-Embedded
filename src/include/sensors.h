#ifndef SENSORS_H
#define SENSORs_H

/* prototypes for test sensor */
int test_sensor(double *ptr);
void init_test(void);	// satisfies init req for testing

/* prototype for FPS_V2_Range 5V pressure sensor */
int fps_v2_range_5v(double *ptr);

#endif
