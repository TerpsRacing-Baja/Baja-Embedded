#include <math.h>
#include "include/sensors.h"
#include "include/blast_data.h"

sensor sensor_table[] = {
	{"TEST", NULL, "Testies", -1, NAN, test_sensor, TEST, NULL},
	{"FPS_V2_RANGE", NULL, "PSI", -1, NAN, fps_v2_range_5v, AIO, NULL},
	{"MLX90614", NULL, "degrees C", -1, NAN, mlx90614, I2C, NULL},
	{NULL, NULL, NULL, -1, NAN, NULL, 0, NULL}
};
