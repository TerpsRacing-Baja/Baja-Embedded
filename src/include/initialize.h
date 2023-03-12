#ifndef INITIALIZE_H
#define INITIALIZE_H

#include <mraa.h>

/**
 * initializes an analog sensor and provides a context given a pin assignment.
 *
 */
int init_aio(mraa_aio_context *aio, unsigned int pin);

/**
 * initializes a digital sensor and provides a context given a pin assignment.
 *
 */
int init_gpio(mraa_gpio_context *gpio, unsigned int pin);

/**
 * "initializes" an i2c sensor. basically just exists to remind me how i2c
 * works, as most initialization work is just hard-coded in update functions.
 *
 */
int init_i2c();

#endif
