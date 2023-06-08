#include <mraa.h>
#include "include/threading.h" // for car model
#include "include/racecapture_sitl.h"
#include "include/racecapture_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef TESTING
static mraa_uart_context uart_contex;
#endif

/* Uses libmraa to prepare a serial port for reading
 * @author AGA
 * @return whether init is successful
 */
int rc_serial_init(void) {
    uart_contex= mraa_uart_init(UART);
    if (uart_contex == NULL) {
        fprintf(stderr, "Failed to initialize UART\n");
        return 0;
    }
    return 1;
}

/* Loops endlessly to get sensor data from the racecapture and push it to the
 * car model. Relies on TBD signal handler to kill the loop before the thread
 * is joined.
 * @author AGA
 */
void rc_serial_read_loop(race_capture *rc_data) {
    states curr_state= READY;
    char *tot_sensor_data;
    int num_sensors;

    for (;;) {
        switch(curr_state) {
            case READY: 
                char start_flag_buf;
                if (mraa_uart_read(uart_contex, &start_flag_buf, 1) && start_flag_buf == START_FlAG) {
                    curr_state= GET_NUM;
                }
                else {
                    printf("No expected start flag.\n");
                }
                break;

            case GET_NUM:
                char num_sensors_buf;
                if (mraa_uart_read(uart_contex, &num_sensors_buf, 1) && num_sensors_buf <= 15) {
                    num_sensors= num_sensors_buf;
                    curr_state= GET_SENSOR;
                }
                else {
                    printf("No expected number of sensors.\n");
                    curr_state= READY;
                }
                break;
            
            case GET_SENSOR:
                char *sensor_buf= malloc(4);
                int read_unsuccessful= 0;
                int i= 0;
                tot_sensor_data= malloc(4 * num_sensors);
                while (i < num_sensors && !read_unsuccessful) {
                    if (mraa_uart_read(uart_contex, sensor_buf, 4) == 4) {
                        strncpy(tot_sensor_data + (i*4), sensor_buf, 4);
                        sensor_buf= realloc(sensor_buf, 4);
                        i++;
                    }
                    else {
                        read_unsuccessful= 1;
                    }
                }

                if (read_unsuccessful) {
                    printf("Unsuccessful read of sensor %d of %d sensors.\n", i, num_sensors);
                    free(tot_sensor_data);
                    curr_state= READY;
                }
                else {
                    curr_state= GET_END_FLAG;
                }
                free(sensor_buf);
                break;
            
            case GET_END_FLAG:
                char end_flag_buf;
                if (mraa_uart_read(uart_contex, &end_flag_buf, 1) && end_flag_buf == END_FLAG) {
                    /* thread-safe write of data in tot_sensor_data buffer */
                }
                else {
                    printf("No expected end flag.\n");
                    free(tot_sensor_data);
                }
                curr_state= READY;
                break;
        } 
    }
}

static uint8_t read_byte(void) {
    #ifdef TESTING
    return sitl_read_byte();
    #endif
}
