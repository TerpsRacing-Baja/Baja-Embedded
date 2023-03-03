#include "race_capture.h"
#include "sensor.h"

typedef struct
{
    race_capture rc;
    sensor sens[];
} car_model;
