#ifndef DISCOVERY_TIMER_INTERFACE_H
#define DISCOVERY_TIMER_INTERFACE_H

#include "stdint.h"
#include "esp_err.h"


typedef void (*discovery_timer_callback)(void);


typedef struct{
        //Start the timer
        esp_err_t (*start_timer)();
        //stop the timer
        esp_err_t (*stop_timer)();
        uint32_t (*get_current_time)();
        //Invoked when timer elapses
        //void (*discovery_timer_callback)(void);
        void (*register_callback)(discovery_timer_callback callback);

}discovery_timer_interface_t;


#endif