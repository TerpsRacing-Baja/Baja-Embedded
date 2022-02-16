#include "include/sensors.h"
#include <stdlib.h>

/**
 * test sensor that generates some random data
 * 
 */
int test_sensor(double *ptr)
{
	srand(time(NULL));	// random seed for dummy data
	*ptr = 20.00 + (rand() / (RAND_MAX / 60));
	return 1;
}
