#ifndef THREADING_H
#define THREADING_H

typedef struct
{
    /* data */
} sensor;

typedef struct
{
    /* data */
} race_capture;

typedef struct
{
    race_capture *rc;
    sensor * sens;
} car_model;

typedef struct {
    car_model *cm;
    locks lkarr;
} args;

typedef pthread_mutex_t * locks;

#endif
