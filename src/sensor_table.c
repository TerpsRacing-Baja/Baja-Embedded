#include "include/sensors.h"
#include "include/blast_data.h"

sensor sensor_table[] = {
	{"TEST", "Testies", test_sensor, TEST, NULL},
	{NULL, NULL, NULL, 0, NULL}
};
