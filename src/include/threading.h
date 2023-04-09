#ifndef THREADING_H
#define THREADING_H

#include <pthread.h>
#include "include/blast_data.h"

typedef struct {
    /* data */
} race_capture;

typedef struct {
    race_capture *rc;
    sensor **sensor_key;
    int num_sensors;
} car_model;

typedef struct {
    car_model *cm;
    pthread_mutex_t *lock_array;
} args;

#endif
