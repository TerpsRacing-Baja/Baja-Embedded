#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include "include/threading.h"

#define SENSOR_COUNT 20

void *start_sp(void *p_)
{

}

void *start_rc(void *p_)
{

}

void *start_fw(void *p_)
{

}

int main(void)
{
    car_model cm;
    locks lock_arr = malloc(SENSOR_COUNT);

    for (int i = 0; i < SENSOR_COUNT; ++i) {
	pthread_mutex_init(&lock_arr[i], NULL);
    }

    args pth_args= {&cm, lock_arr};

    pthread_t sensor_process, rc_process, file_write;
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);

    pthread_create(&sensor_process, &tattr, start_sp, (void*) &pth_args);
    pthread_create(&rc_process, &tattr, start_rc, (void*) &pth_args);
    pthread_create(&file_write, &tattr, start_fw, (void*) &pth_args);

    /* free lock array and join threads */
    pthread_join(sensor_process, NULL);
    pthread_join(rc_process, NULL);
    pthread_join(file_write, NULL);

    for (int i = 0; i < SENSOR_COUNT; ++i) {
	pthread_mutex_destroy(&lock_arr[i]);
    }

    free(lock_arr);
}
