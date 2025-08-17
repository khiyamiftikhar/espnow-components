#ifndef ESPNOW_DISCOVERY_H
#define ESPNOW_DISCOVERY_H

#include "stdint.h"
#include "stdbool.h"
#include "esp_err.h"
#include "discovery_timer_interface.h"
typedef struct{

        //Start the Discovery
        esp_err_t (*send_discovery)();
        //Get informed on incoming discovery request
        void (*process_discovery_callback)(const uint8_t *mac_addr);
        //Acknowldge the discovery to the device which broadasted
        esp_err_t (*acknowledge_the_discovery)(const uint8_t *mac_addr);
        //GEt informed when the discovery is acknowledged so that discovery is stopped
        void (*process_discovery_acknowledgement_callback)(const uint8_t *mac_addr);
        esp_err_t (*add_peer)(const uint8_t *mac_addr);
}espnow_discovery_interface_t;


typedef struct{

        //Check if any deivce is added to the white list
        bool (*is_white_listed)(const uint8_t *mac_addr);

}discovery_whitelist_interface_t;



/*To trigger the starting of discovery*/
typedef struct{

        void(*button_event_callback)(); //Called when button is pressed

}discovery_input_trigger_interface_t;


typedef struct{
        
        //Total duration to run the discovery boradcast
        uint32_t discovery_duration;            //mircoseconds
        ///Interval between each discoovery broadcast. Must be less than discovery_duration
        uint32_t discovery_interval;             //mircoseconds
        espnow_discovery_interface_t* discovery;
        discovery_whitelist_interface_t* whitelist;
        discovery_timer_interface_t* timer;

}config_espnow_discovery;





esp_err_t discovery_service_init(config_espnow_discovery* config);

//Now publically available
//Earlier called witjhin discovery_init, but causing crashes because discovery
//callbacks are divided between discovery and message services , and the others
//are not assigned when discovery was called so crashing
void start_discovery();

#endif