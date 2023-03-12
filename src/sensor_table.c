#include "include/sensors.h"
#include "include/blast_data.h"

sensor sensor_table[] = {
	{"TEST", NULL, "Testies", -1, test_sensor, TEST, NULL},
	{"FPS_V2_RANGE", NULL, "PSI", -1, fps_v2_range_5v, AIO, NULL},
	{"MLX90614", NULL, "degrees C", -1, mlx90614, I2C, NULL},
	{NULL, NULL, NULL, -1, NULL, 0, NULL}
};
