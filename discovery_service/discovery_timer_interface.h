#ifndef DISCOVERY_TIMER_INTERFACE_H
#define DISCOVERY_TIMER_INTERFACE_H

#include "stdint.h"
#include "esp_err.h"





typedef struct{
        
        //These are the methods that discovery service needs to call
        //Start the timer
        esp_err_t (*start_timer)();
        //stop the timer
        esp_err_t (*stop_timer)();
        uint32_t (*get_current_time)();
        //Invoked when timer elapses
        
        //void (*register_callback)(discovery_timer_callback callback);

}discovery_timer_interface_t;


//This is the callback it needs to supply to a invoker
typedef void (*discovery_timer_callback)(void);



#endif