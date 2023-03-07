#include "p_thread_args.h"
#include <stdlib.h>


void *start_sp(void *p_){

}
void *start_rc(void *p_){

}
void *start_fw(void *p_){

}

int main(void) {
    car_model cm;
    locks lock_arr = malloc(20);

    args pth_args= {&cm, lock_arr};
    
    pthread_t sensor_process, rc_process, file_write;
    pthread_attr_t tattr;

    pthread_create(&sensor_process, &tattr, start_sp, (void*) &pth_args);
    pthread_create(&rc_process, &tattr, start_rc, (void*) &pth_args);
    pthread_create(&file_write, &tattr, start_fw, (void*) &pth_args);

    // free lock array and join threads 
    (void) pthread_join(sensor_process, NULL);
    (void) pthread_join(rc_process, NULL);
    (void) pthread_join(file_write, NULL);
    
    free(lock_arr);
}