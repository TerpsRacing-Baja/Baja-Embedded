#ifndef INITIALIZE_H
#define INITIALIZE_H

int init_aio(mraa_aio_context aio, unsigned int pin);
int init_gpio(mraa_gpio_context gpio, unsigned int pin);

#endif
