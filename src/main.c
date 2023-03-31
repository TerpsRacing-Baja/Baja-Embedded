#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "include/blast_data.h"
#include "include/threading.h"
#include "include/sensors.h"

#define SENSOR_COUNT 20

static void *start_sp(void *p)
{
    args *args = p;

    for (;;) {
        for (int i = 0; i < SENSOR_COUNT; ++i) {
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
 * @param p_ pthreads function, meant to be a pointer to an argument structure
 * @return required by pthreads, will not be a meaningful value
 */
void *start_rc(void *p)
{
    // TODO: AGA still writing the actual racecapture logic
}

static void *start_fw(void *args)
{

}

int main(void)
{
    car_model cm;
    locks lock_arr = malloc(SENSOR_COUNT);

    /* initialize sensor array */
    /* just hard-code test sensors for now */
    cm.sensor_key = malloc(sizeof(sensor *) * SENSOR_COUNT);

    for (int i = 0; i < SENSOR_COUNT; ++i) {
	    cm.sensor_key[i] = build_sensor("TEST", "TEST", "TESTIES", test_sensor, TEST, 0);
    }

    for (int i = 0; i < SENSOR_COUNT; ++i) {
	    pthread_mutex_init(&lock_arr[i], NULL);
    }

    args pth_args= {&cm, lock_arr};

    pthread_t sensor_process, rc_process, file_write;
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);

    pthread_create(&sensor_process, &tattr, start_sp, (void *) &pth_args);
    pthread_create(&rc_process, &tattr, start_rc, (void *) &pth_args);
    pthread_create(&file_write, &tattr, start_fw, (void *) &pth_args);

    // AGA: TODO add some kind of signal to kill any infinite loops beforehand

    /* free lock array and join threads */
    pthread_join(sensor_process, NULL);
    pthread_join(rc_process, NULL);
    pthread_join(file_write, NULL);

    for (int i = 0; i < SENSOR_COUNT; ++i) {
	    pthread_mutex_destroy(&lock_arr[i]);
    }

    /* tear down sensor structs */
    for (int i = 0; i < SENSOR_COUNT; ++i) {
	    destroy_sensor(cm.sensor_key[i]);
    }

    free(lock_arr);
}
