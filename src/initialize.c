#include <mraa.h>
#include "include/initialize.h"

/**
 * initialiation functions for different pin types on the edison.
 * these are meant to be mapped to each sensor interface's
 * initialization function pointers.
 * 
 */

int init_aio(mraa_aio_context aio, unsigned int pin)
{
	aio = mraa_aio_init(pin);

	if (aio == NULL) {
		mraa_deinit();
		return -1;
	}

	return 0;
}

int init_gpio(mraa_gpio_context gpio, unsigned int pin)
{
	gpio = mraa_gpio_init(pin);

	if (gpio == NULL) {
		mraa_deinit();
		return -1;
	}

	return 0;
}

/*

solutions:
	- add context pointer to struct, somehow track + cast it
	- different structures for distinct sensor types
		-> use composition for integral aspects


*/
