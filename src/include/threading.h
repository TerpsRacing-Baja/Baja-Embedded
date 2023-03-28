#ifndef THREADING_H
#define THREADING_H

#include "include/blast_data.h"

typedef struct {
    /* data */
} race_capture;

typedef struct {
    race_capture *rc;
    sensor **sensor_key;
} car_model;

typedef struct {
    car_model *cm;
    locks lkarr;
} args;

typedef pthread_mutex_t * locks;

#endif
