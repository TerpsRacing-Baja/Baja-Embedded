#include <mraa.h>
#include <stdio.h>

int main()
{
	int mux = 0b1010;
	for (;;) {
		mraa_gpio_context multi_pin;
		int pins[] = {10, 11, 12, 13};
		int out[] = {mux & 0b1, mux &0b10, mux & 0b100, mux & 0b1000};

		/* initialize pins */
		if ((multi_pin = mraa_gpio_init_multi(pins, 4)) == NULL) {
			fprintf(stderr, "Failed to initialize GPIO pins 10, 11, 12, 13\n");
		}

		/* set pin directions */
		if (mraa_gpio_dir(multi_pin, MRAA_GPIO_OUT) != MRAA_SUCCESS) {
			fprintf(stderr, "Failed to set pins 10, 11, 12, 13 to GPIO_OUT\n");
		}

		/* write values out to mux */
		mraa_gpio_write_multi(multi_pin, out);

		/* does this turn off the pins ? */
		mraa_gpio_close(multi_pin);

		sleep(3);
	}
}
