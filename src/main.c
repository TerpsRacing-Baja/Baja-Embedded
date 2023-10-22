#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/time.h>
#include "include/blast_data.h"
#include "include/threading.h"
#include "include/sensors.h"
#include "include/racecapture_interface.h"
#include "include/arduino-serial-lib.h"

mraa_i2c_context i2c;
mraa_gpio_context pin10, pin11, pin12, pin13;
int serial_fd;

unsigned int r_seed;    // used for reentrant seeding of test sensor value generation

static void *start_sp(void *p)
{
    args *args = p;

    #ifdef TESTING
        r_seed = time(NULL);    // initial test sensor seed
    #endif

    /* constantly loop through all sensors, acquiring most recent values (and locking accordingly) */
    for (;;) {
        for (int i = 0; i < args->cm->num_sensors; ++i) {
            pthread_mutex_lock(&args->lock_array[i]);
            sensor *sens = args->cm->sensor_key[i];
            sens->update(&sens->val, sens->context);
            pthread_mutex_unlock(&args->lock_array[i]);
        }
    }
}

/* The racecapture is an event loop, the lifecycle is specified here for
 * clarity, while the code is in a separate file for brevity
 * @author AGA
 * @param p pthreads function, meant to be a pointer to an argument structure
 * @return required by pthreads, will not be a meaningful value
 */
void *start_rc(void *p)
{
    int init_succeeded;
    args *args = p;

    init_succeeded = !rc_serial_init(); // side effect of actually doing the initialization

    if (init_succeeded)
    {
        #ifdef TESTING
        //rc_serial_read_loop(args->cm->rc);
        //sitl();
        #endif
    }
    else {
        fprintf(stderr, "[RC-THREAD]: UART initialization FAILED .. racecapture thread ended");
    }
}

static void *start_fw(void *p)
{
    int sd_exists;              // set true if there is an sd card
    char filename[70] = "/mnt"; // name of log file
    FILE *logfile;              // file pointer to logfile
    struct timeval tp;          // timeval struct for holding current time info
    struct timespec req;        // timespec struct for requesting time from nanosleep
    data_msg msg;               // store most recent rich sensor data

    args *args = p;
    sensor **sensor_key = args->cm->sensor_key;

    req.tv_sec = 0;
    req.tv_nsec = SAMPLE_PERIOD * 1000000L;

    /* do an access and mount sd if it exists */
	/* open file based on datetime */
	if (access("/dev/mmcblk1p1", W_OK) == 0) {
		sd_exists = 1;
		if (mount("/dev/mmcblk1p1", "/mnt", "vfat", MS_NOATIME, NULL) == 0) {
			time_t raw_time;
			time(&raw_time);
			struct tm *tm = localtime(&raw_time);
			strftime(filename + 5, sizeof(filename) - 5, "%Y-%m-%d_%H-%M-%S", tm);

			logfile = fopen(filename, "ab+");

			fwrite("Label|Name|Unit|Timestamp|Value\n", 32, 1, logfile);
		} else {
			fprintf(stderr, "Failed to mount SD card\n");
			sd_exists = 0;
		}
	} else {
		sd_exists = 0;
	}

    #ifdef DEBUG
        printf("[FILE-WRITE THREAD] data collection loop:\n");
        fflush(stdout);
    #endif

    /* acquire and save current values from everything in the car model according to sample period */
    for (;;) {
        for (int i = 0; i < args->cm->num_sensors; ++i) {
            pthread_mutex_lock(&args->lock_array[i]);

            gettimeofday(&tp, NULL);
            msg = build_msg(sensor_key[i]->label, sensor_key[i]->name,
                    sensor_key[i]->unit,
                    ((unsigned long long)tp.tv_sec * 1000) + (tp.tv_usec / 1000),
                    sensor_key[i]->val);

            pthread_mutex_unlock(&args->lock_array[i]);
            char *msg_string = stringify_msg(msg);

            #ifdef DEBUG
                printf("[FILE-WRITE THREAD] (%d) %s\n", i, msg_string);
                fflush(stdout);
            #endif

            if (sd_exists) {
                fwrite(msg_string, strlen(msg_string), 1, logfile);
                fwrite("\n", 1, 1, logfile);
                fflush(logfile);
            }

            /* also write message to radio */
            // should be able to do this pretty easily since the radio modules
            // are actually treated as virtual serial ports, not usb ports
            // just need to open correct dev file and use some linux builtins to
            // set baud rate and some other io settings
            // see: https://github.com/todbot/arduino-serial/tree/main
            serialport_write(serial_fd, msg_string);
            serialport_write(serial_fd, "\n");

            free(msg_string);
        }

        nanosleep(&req, NULL);
    }
}

int main(void)
{
    car_model cm;                                       // shared model of the car's state
    pthread_mutex_t *lock_arr;                          // array of mutexes for edison sensors
    pthread_t sensor_process, rc_process, file_write;   // thread variables
    pthread_attr_t tattr;                               // thread attributes (we don't use)
    args pth_args;                                      // argument passed into each thread
    FILE *config_file;                                  // pointer to confguration file
    char *config = malloc(5096);                        // buffer for holding config info

    // TODO: threaded signal handling (sigint, sigterm, killing threads)

    /* create an i2c context for the edison - exposed on bus 6 */
    i2c = mraa_i2c_init(6);

    /* pin initializations - only runs if compiling on edison platform */
    #ifndef TESTING
	    if (i2c == NULL) {
			fprintf(stderr, "profound meditation: failed to initialize i2c-6 bus\n");
			mraa_deinit();
			exit(-1);
		}

		/* we will be using digital pins 10, 11, 12, and 13 */
		if (!(pin10 = mraa_gpio_init(10))) {
			fprintf(stderr, "Failed to initialize GPIO pin 10\n");
		}

		if (!(pin11 = mraa_gpio_init(11))) {
			fprintf(stderr, "Failed to initialize GPIO pin 11\n");
		}

		if (!(pin12 = mraa_gpio_init(12))) {
			fprintf(stderr, "Failed to initialize GPIO pin 12\n");
		}

		if (!(pin13 = mraa_gpio_init(13))) {
			fprintf(stderr, "Failed to initialize GPIO pin 13\n");
		}

		if (mraa_gpio_dir(pin10, MRAA_GPIO_OUT) != MRAA_SUCCESS) {
			fprintf(stderr, "Failed to set GPIO pin 10 to output mode\n");
		}

		if (mraa_gpio_dir(pin11, MRAA_GPIO_OUT) != MRAA_SUCCESS) {
			fprintf(stderr, "Failed to set GPIO pin 11 to output mode\n");
		}

		if (mraa_gpio_dir(pin12, MRAA_GPIO_OUT) != MRAA_SUCCESS) {
			fprintf(stderr, "Failed to set GPIO pin 12 to output mode\n");
		}

		if (mraa_gpio_dir(pin13, MRAA_GPIO_OUT) != MRAA_SUCCESS) {
			fprintf(stderr, "Failed to set GPIO pin 13 to output mode\n");
		}
	#endif

    serial_fd = serialport_init("/dev/ttyACM0", 9600);

    /* read sensor configuration information from config file */
    // TODO: flash config via usb serial (not a code task tho lol)
    config_file = fopen("./configuration", "r");
    fread(config, 1, 5096, config_file);

    cm.num_sensors = configure_sensors(config, &cm.sensor_key);
    free(config);

    /* initialize locks */
    lock_arr = malloc(sizeof(pthread_mutex_t) * cm.num_sensors);

    for (int i = 0; i < cm.num_sensors; ++i) {
	    pthread_mutex_init(&lock_arr[i], NULL);
    }

    pthread_attr_init(&tattr);

    /* load pthread arguments */
    pth_args.cm = &cm;
    pth_args.lock_array = lock_arr;

    pthread_create(&sensor_process, &tattr, start_sp, (void *) &pth_args);
    pthread_create(&rc_process, &tattr, start_rc, (void *) &pth_args);
    pthread_create(&file_write, &tattr, start_fw, (void *) &pth_args);

    // AGA: TODO add some kind of signal to kill any infinite loops beforehand

    /* free lock array and join threads */
    pthread_join(sensor_process, NULL);
    pthread_join(rc_process, NULL);
    pthread_join(file_write, NULL);

    for (int i = 0; i < cm.num_sensors; ++i) {
	    pthread_mutex_destroy(&lock_arr[i]);
    }

    /* tear down sensor structs */
    for (int i = 0; i < cm.num_sensors; ++i) {
	    destroy_sensor(cm.sensor_key[i]);
    }

    free(lock_arr);
    serialport_close(serial_fd);
}
