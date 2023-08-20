#include <mraa.h>
#include "racecapture_sitl.h"

#define S_FLAG 0xF0 
#define E_FLAG 0xC5 

const float test_reading = 3.545;
const uint8_t *test_reading_deconstructed = (uint8_t *) &test_reading;
uint8_t *sitl_rc_msg[7];

void init_sitl_rc(void) {
    sitl_rc_msg[0] = S_FLAG;
    sitl_rc_msg[1] = 1;
    sitl_rc_msg[2] = test_reading_deconstructed[0];
    sitl_rc_msg[3] = test_reading_deconstructed[1];
    sitl_rc_msg[4] = test_reading_deconstructed[2];
    sitl_rc_msg[5] = test_reading_deconstructed[3];
    
}

uint8_t sitl_read_byte(void) {
    return '\0';
}
