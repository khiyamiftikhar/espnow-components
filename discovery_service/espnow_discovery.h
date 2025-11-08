#ifndef ESPNOW_DISCOVERY_H
#define ESPNOW_DISCOVERY_H

#include "stdint.h"
#include "stdbool.h"
#include "esp_err.h"
#include "discovery_timer_interface.h"
#include "peer_manager_interface.h"
#include "database_interface.h"
#include "discovery_interface.h"
#include "event_system_adapter.h"




DECLARE_EVENT_ADAPTER(DISCOVERY_SERVICE);
//The below is  equivalent 
//ESP_EVENT_DECLARE_BASE(MY_MODULE_NAME_ROUTINE_EVENT_BASE);
//extern const char* const DISCOVERY_SERVICE_ROUTINE_EVENT_BASE;
//The below is  equivalent 
//ESP_EVENT_DECLARE_BASE(MY_MODULE_NAME_EXCEPTION_EVENT_BASE);
//extern const char* const DISCOVERY_SERVICE_EXCEPTION_EVENT_BASE;



//The events related to discovery
#define   DISCOVERY_EVENT_DISCOVERY_COMPLETE            1

        



typedef struct{
        
        //Total duration to run the discovery boradcast
        uint32_t discovery_duration;            //mircoseconds
        ///Interval between each discoovery broadcast. Must be less than discovery_duration
        uint32_t discovery_interval;             //mircoseconds
        esp_now_transport_discovery_interface_t* discovery_interface;
        esp_now_peer_manager_interface_t* peer_manager_interface;
        database_interface_t* database_interface;
      
}config_espnow_discovery;




typedef enum{
        DISCOVERY_EVENT_DISCOVERY_MESSAGE_ARRIVED=0,
        DISCOVERY_EVENT_DISCOVERY_MESSAGE_ACK_ARRIVED,

}discovery_events_t;




/// @brief Handles the above enum evens
/// @param event 
/// @param src_mac 
//void discovery_events_handler(discovery_events_t event,uint8_t* src_mac);

esp_err_t discovery_service_init(config_espnow_discovery* config);

//Now publically available
//Earlier called witjhin discovery_init, but causing crashes because discovery
//callbacks are divided between discovery and message services , and the others
//are not assigned when discovery was called so crashing
void start_discovery();

#endif