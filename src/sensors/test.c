#include "include/sensors.h"
#include <stdlib.h>
#include <time.h>

int test_sensor(float *ptr, mraa_context context)
{
	srand(time(NULL));	// random seed for dummy data
	*ptr = 20.00 + (rand() / (RAND_MAX / 60));
	return 1;
}
