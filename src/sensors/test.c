#include "include/blast_data.h"
#include "include/sensors.h"
#include <stdlib.h>
#include <time.h>

int test_sensor(float *ptr, mraa_context context)
{
	*ptr = 20.00 + (rand_r(&r_seed) / (RAND_MAX / 60));
	return 1;
}
