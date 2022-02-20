#include <mraa.h>
#include "include/sensors.h"
#include "include/blast_data.h"

int fps_v2_range_5v(double *ptr, mraa_context context)
{
	mraa_aio_context aio = (mraa_aio_context)context;

	if ((*ptr = mraa_aio_read_float(aio)) < 0) {
		return -1;
	} else {
		/* calibrate */
		return 1;
	}

}
