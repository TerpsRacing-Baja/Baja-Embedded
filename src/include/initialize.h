#ifndef INITIALIZE_H
#define INITIALIZE_H

/**
 * initializes an analog sensor and provides a context given a pin assignment
 */
int init_aio(mraa_aio_context *aio, unsigned int pin);

/**
 * initializes a digital sensor and provides a context given a pin assignment
 */
int init_gpio(mraa_gpio_context *gpio, unsigned int pin);

#endif
