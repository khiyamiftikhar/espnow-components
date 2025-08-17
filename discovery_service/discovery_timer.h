#ifndef DISCOVERY_TIMER_H
#define DISCOVERY_TIMER_H


#include "discovery_timer_interface.h"





/*
typedef struct{

    uint32_t interval;  //timer expie
    uint32_t duration;

}discovery_timer_config_t;
*/


discovery_timer_interface_t* timer_create(uint32_t duration);
#endif