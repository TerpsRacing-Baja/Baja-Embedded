#include "include/sensors.h"
#include "include/blast_data.h"

sensor sensor_table[] = {
	{"TEST", NULL, "Testies", test_sensor, TEST, NULL},
	{"FPS_V2_RANGE", NULL, "PSI", fps_v2_range_5v, AIO, NULL},
	{"MLX90614", NULL, "degrees C", mlx90614, I2C, NULL},
	{NULL, NULL, NULL, NULL, 0, NULL}
};
