#include <mraa.h>
#include "include/initialize.h"

/**
 * initialiation functions for different pin types on the edison.
 * these are mapped according to each sensor interface's enumerated
 * sensor type.
 */

int init_aio(mraa_aio_context *aio, unsigned int pin)
{
	*aio = mraa_aio_init(pin);

	/* only compile on edison platform */
	#ifndef TESTING
		if (*aio == NULL) {
			mraa_deinit();
			return -1;
		}
	#endif

	return 0;
}

int init_gpio(mraa_gpio_context *gpio, unsigned int pin)
{
	*gpio = mraa_gpio_init(pin);

	/* only compile on edison platform */
	#ifndef TESTING
		if (*gpio == NULL) {
			mraa_deinit();
			return -1;
		}
	#endif

	return 0;
}

int init_i2c()
{
	/**
	 * We do nothing here. The I2C context is global since we can host
	 * multiple slaves, and addressing is configured in individual sensor
	 * update functions. I'm only leaving this function here for the sake
	 * of consistency in the sensor interface.
	 * 
	 */

	return 0;
}
