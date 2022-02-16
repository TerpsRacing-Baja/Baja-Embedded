#include "include/sensors.h"
#include <stdlib.h>
#include <time.h>

int test_sensor(double *ptr)
{
	srand(time(NULL));	// random seed for dummy data
	*ptr = 20.00 + (rand() / (RAND_MAX / 60));
	return 1;
}
