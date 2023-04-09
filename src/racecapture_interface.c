#include <mraa.h>
#include "include/threading.h" // for car model

#ifndef TESTING
static mraa_uart_context uart_contex;
#endif

/* Uses libmraa to prepare a serial port for reading
 * @author AGA
 * @return whether init is successful
 */
int rc_serial_init(void) {
    return 0;
}

/* Loops endlessly to get sensor data from the racecapture and push it to the
 * car model. Relies on TBD signal handler to kill the loop before the thread
 * is joined.
 * @author AGA
 */
void rc_serial_read_loop(race_capture *rc_data) {
    
}

static uint8_t read_byte(void) {
    #ifdef TESTING
        
    #endif
}
