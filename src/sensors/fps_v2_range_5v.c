#include <mraa.h>
#include "include/sensors.h"
#include "include/blast_data.h"

int fps_v2_range_5v(float *ptr, mraa_context context)
{
	mraa_aio_context aio = (mraa_aio_context)context;

	if ((*ptr = mraa_aio_read_float(aio)) < 0) {
		return -1;
	} else {
		/* convert to bar on 0-10 scale, then to psi */
		*ptr = ((2.5 * (*ptr)) - 1.25) * 14.504;
		return 1;
	}
}
