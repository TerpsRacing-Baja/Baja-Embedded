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

#define SENSOR_COUNT 20

mraa_i2c_context i2c;
mraa_gpio_context pin10, pin11, pin12, pin13;

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
        rc_serial_read_loop(args[0].cm->rc);
    } 
    else
        fprintf(stderr, "[RC-THREAD]: UART initialization FAILED .. racecapture thread ended");
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

            free(msg_string);
            pthread_mutex_unlock(&args->lock_array[i]);
        }

        nanosleep(&req, NULL);
    }
}

int main(void)
{
    car_model cm;                                       // shared model of the car's state
    pthread_mutex_t *lock_arr = malloc(SENSOR_COUNT);   // array of mutexes for edison sensors
    pthread_t sensor_process, rc_process, file_write;   // thread variables
    pthread_attr_t tattr;                               // thread attributes (we don't use)
    args pth_args= {&cm, lock_arr};                     // argument passed into each thread

    /* initialize sensor array */
    /* just hard-code test sensors for now */
    cm.sensor_key = malloc(sizeof(sensor *) * SENSOR_COUNT);

    for (int i = 0; i < SENSOR_COUNT; ++i) {
	    cm.sensor_key[i] = build_sensor("TEST", "TEST", "TESTIES", test_sensor, TEST, 0);
    }

    cm.num_sensors = SENSOR_COUNT;

    for (int i = 0; i < cm.num_sensors; ++i) {
	    pthread_mutex_init(&lock_arr[i], NULL);
    }

    pthread_attr_init(&tattr);

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
}
