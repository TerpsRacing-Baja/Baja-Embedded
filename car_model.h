#ifndef CAR_MODEL_H
#define CAR_MODEL_H
#include "race_capture.h"
#include "sensor.h"

typedef struct
{
    race_capture *rc;
    sensor * sens;
} car_model;

#endif
