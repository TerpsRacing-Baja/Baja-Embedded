#ifndef THREADING_H
#define THREADING_H

#include "include/blast_data.h"

typedef struct {
    /* data */
} race_capture;

typedef struct {
    race_capture *rc;
    sensor **sensor_key;
    int num_sensors;
} car_model;

typedef pthread_mutex_t * locks;

typedef struct {
    car_model *cm;
    locks lock_array;
} args;

#endif
