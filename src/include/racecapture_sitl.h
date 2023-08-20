#ifndef RACECAPTURE_SITL_H
#define RACECAPTURE_SITL_H

#include <mraa.h>

void sitl_read_byte(void *msg);
int sitl(void);
uint8_t generate_onebyte_msg(void);
uint32_t generate_fourbyte_msg(void);

#endif