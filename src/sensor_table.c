#include "include/sensors.h"
#include "include/blast_data.h"

sensor sensor_table[] = {
	{"TEST", "Testies", test_sensor, TEST, NULL},
	{"FPS_V2_RANGE", "PSI", fps_v2_range_5v, AIO, NULL},
	{NULL, NULL, NULL, 0, NULL}
};
