#ifndef P_THREAD_ARGS_H
#define P_THREAD_ARGS_H
#include "locks.h"
#include "car_model.h"

typedef struct {
    car_model *cm;
    locks lkarr;   
} args;

#endif