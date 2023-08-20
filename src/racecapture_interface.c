#include <mraa.h>
#include "include/threading.h" // for car model
#include "include/racecapture_sitl.h"
#include "include/racecapture_interface.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static mraa_uart_context uart_contex;
static states curr_state;

#ifndef TESTING
#include "include/racecapture_sitl.h" // for testing function prototypes
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

#define UART 0      // keeping this for when we can test the mraa context directly

static states expected_state;
curr_state= READY;

int sitl(void) {
    pid_t child_pid;
    int status;
    uint32_t msg;
    if (curr_state == READY || curr_state == GET_NUM || curr_state == GET_END_FLAG) {
        msg= generate_onebyte_msg();
        if (msg == START_FLAG) {
            if (curr_state == READY) {
                expected_state= GET_NUM;
            }
            else {
                curr_state= READY;
            }
        }
        else if (msg == END_FLAG) {     // this conditional isn't necessary but is more so for clarity (especially once threading has been implemented)
            if (curr_state == GET_END_FLAG) {
                expected_state= READY;
            }
            else {
                curr_state= READY;
            }
        }
        else if (msg <= 15) {
            if (curr_state == GET_NUM) {
                expected_state= GET_SENSOR;
            }
            else {
                curr_state= READY;
            }
        }
        else {
            expected_state= READY;
        }
    }
    else {  
        msg= generate_fourbyte_msg();
        expected_state= GET_END_FLAG;
    }

    child_pid= fork();

    if (child_pid >= 0) { // fork succeeded
        if (child_pid == 0) { // parent process 
            wait(&status);
            /* compares curr state as reported by child process to expected state *
                and updates expected state */
            if (curr_state == expected_state) {
                printf("SUCCESSFUL: Current state matches expected state (%d)\nData read: %d\n\n", curr_state, msg);
                return sitl();
            }
            else {
                printf("TERMINATED: Current state does not match expected state\nExpected state: %d\nCurrent state: %d\nData read: %d\n\n", expected_state, curr_state, msg);
            }
        }

        else { // child process
            sitl_read_byte(&msg);
        }
    }
    else {
        perror("Unsuccesful fork\n");
        return 0;
    }
}

uint8_t generate_onebyte_msg(void) {
    uint8_t arr[]= {START_FLAG, END_FLAG, 12, 34};
    int r;
    srand(time(NULL));
    r= rand() % sizeof(arr); 
    return *(arr + r);
}

uint32_t generate_fourbyte_msg(void) {
    uint32_t r;
    srand(time(NULL));
    r= rand() % 16;
    return r;
}

void sitl_read_byte(void *msg) {        // "crude shadowing function" that works like rc_serial_read_loop without the infinite loop nor MRAA native functions and structs
    uint32_t data;
    if (curr_state == GET_SENSOR) {
        data= *((uint32_t *) msg);
    }
    else {
        data= *((uint8_t *) msg);
    }
    switch (curr_state) {
        case READY:
            if (data == START_FLAG) {
                curr_state= GET_NUM;
            }
            break;
        case GET_NUM:
            if (data <= 15) {
                curr_state= GET_SENSOR;
            }
            break;
        case GET_SENSOR:
            curr_state= GET_END_FLAG;
            break;
        case GET_END_FLAG:
            if (data == END_FLAG) {
                curr_state= READY;
            }
            else {    
                printf("Message has been compromised: end flag not found (found %d)\n", data);
            }
            break;
    }
}
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
    char *tot_sensor_data;
    int num_sensors;
    curr_state= READY;

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
