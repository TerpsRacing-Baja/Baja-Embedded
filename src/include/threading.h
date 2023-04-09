#ifndef THREADING_H
#define THREADING_H

#include <pthread.h>
#include "include/blast_data.h"

typedef struct race_capture {
    /* data */
    // AGA: we can get away with one big lock
    sensor *rc_sensors;
    int num_rc_sensors;
    pthread_mutex_t rc_data_lock;
} race_capture;

typedef struct car_model {
    race_capture *rc;
    sensor **sensor_key;
    int num_sensors;
} car_model;

typedef struct args {
    car_model *cm;
    pthread_mutex_t *lock_array;
} args;

#endif
