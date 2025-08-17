#ifndef DISCOVERY_TIMER_H
#define DISCOVERY_TIMER_H


#include "discovery_timer_interface.h"





/*
typedef struct{

    uint32_t interval;  //timer expie
    uint32_t duration;

}discovery_timer_config_t;
*/

typedef struct{
    discovery_timer_interface_t methods;
    discovery_timer_callback callback_handler;

}discovery_timer_implementation_t;




discovery_timer_implementation_t* timer_create(uint32_t duration);
#endif