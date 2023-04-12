#ifndef RACECAPTURE_H
#define RACECAPTURE_H

#include "threading.h"

#define START_FLAG 0xF0
#define END_FLAG 0XC5

int rc_serial_init(void);
void rc_serial_read_loop(race_capture *rc_data);

#endif