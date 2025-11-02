#ifndef ESPNOW_DISCOVERY_H
#define ESPNOW_DISCOVERY_H

#include "stdint.h"
#include "stdbool.h"
#include "esp_err.h"
#include "event_system_adapter.h"
#include "discovery_timer_interface.h"



DECLARE_EVENT_ADAPTER(DISCOVERY_SERVICE);
//The below is  equivalent 
//ESP_EVENT_DECLARE_BASE(MY_MODULE_NAME_ROUTINE_EVENT_BASE);
//extern const char* const DISCOVERY_SERVICE_ROUTINE_EVENT_BASE;
//The below is  equivalent 
//ESP_EVENT_DECLARE_BASE(MY_MODULE_NAME_EXCEPTION_EVENT_BASE);
//extern const char* const DISCOVERY_SERVICE_EXCEPTION_EVENT_BASE;



//The events related to discovery
#define   DISCOVERY_EVENT_DISCOVERY_COMPLETE            1

/*These are the interfaces which it requires, i.e its dependecies*/
typedef struct{

        //Start the Discovery
        esp_err_t (*send_discovery)();
        
        //Acknowldge the discovery to the device which broadasted
        esp_err_t (*acknowledge_the_discovery)(const uint8_t *mac_addr);
        
        esp_err_t (*add_peer)(const uint8_t *mac_addr);
        
        bool (*is_peer_exist)(const uint8_t *mac_addr);
        //Does not fit in this because it is th odd one. but has to be added so that it must inform on discovery completion
        void (*process_discovery_completion_callback)(uint8_t total_devices_found);
}discovery_comm_interface_t;

        

typedef struct{

        //Check if any deivce is added to the white list
        bool (*is_white_listed)(const uint8_t *mac_addr);

}discovery_whitelist_interface_t;



typedef struct{
        
        //Total duration to run the discovery boradcast
        uint32_t discovery_duration;            //mircoseconds
        ///Interval between each discoovery broadcast. Must be less than discovery_duration
        uint32_t discovery_interval;             //mircoseconds
        discovery_comm_interface_t* discovery;
        discovery_whitelist_interface_t* whitelist;
      
}config_espnow_discovery;




typedef enum{
        DISCOVERY_EVENT_DISCOVERY_MESSAGE_ARRIVED=0,
        DISCOVERY_EVENT_DISCOVERY_MESSAGE_ACK_ARRIVED,

}discovery_events_t;




/// @brief Handles the above enum evens
/// @param event 
/// @param src_mac 
void discovery_events_handler(discovery_events_t event,uint8_t* src_mac);

esp_err_t discovery_service_init(config_espnow_discovery* config);

//Now publically available
//Earlier called witjhin discovery_init, but causing crashes because discovery
//callbacks are divided between discovery and message services , and the others
//are not assigned when discovery was called so crashing
void start_discovery();

#endif