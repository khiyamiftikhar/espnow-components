
#include "esp_log.h"
#include "espnow_discovery.h"
#include "inttypes.h"

#define         DISCOVERY_DURATION          5000    //ms

static const char* TAG= "discovery";


static struct{
    //Total duration to run the discovery boradcast
    uint32_t discovery_duration;            //milliseconds
    ///Interval between each discoovery broadcast. Must be less than discovery_duration
    uint32_t discovery_interval;             //milliseconds
    uint32_t discovery_start_time;
    //These are the interfaces that it uses and are injected to it
    discovery_comm_interface_t* message_interface;
    discovery_whitelist_interface_t* whitelist;
    discovery_timer_interface_t* timer;
    //These are the callbacks that are assigned internally and externally to appropriate invokers
    discovery_service_interface_t discovery_callback_handlers;
}discovery_service={0};




/// @brief Will be invoked when there is some incoming discovery packet
/// Checks if sender is in whitelist and adds as peer, ignores otherwise
/// @param mac_addr 
static void incoming_discovery_handler(const uint8_t *mac_addr){


    //Check if device is in the white list
    if(discovery_service.whitelist->is_white_listed(mac_addr)==true){
        //if yes then add as peer
        discovery_service.message_interface->add_peer(mac_addr);
        //And tell the device that its discoveery was  received, so that it stops if desires
        discovery_service.message_interface->acknowledge_the_discovery(mac_addr);
    }

}


/// @brief Invoked when an acknowledgment message of discovery arrives
/// @param mac_addr 
/// @return 
static void discovery_acknowledgement_handler(const uint8_t *mac_addr){
    if(discovery_service.whitelist->is_white_listed(mac_addr)==true){
            //if yes then add as peer
            discovery_service.message_interface->add_peer(mac_addr);
    }
    

    

}


/// @brief Invoked after discovery interval
static  void timer_elapsed_handler(){

    //Stop the discovery
    uint32_t current_time=discovery_service.timer->get_current_time();
    uint32_t discovery_start_time=discovery_service.discovery_start_time;
    uint32_t duration=discovery_service.discovery_duration;
    //If time passed is less than the discovery duration than 
ESP_LOGI(TAG, "current time %"PRIu32" previous time %"PRIu32 "duration%"PRIu32, current_time, discovery_start_time,duration); 
   if((current_time-discovery_start_time)<duration){
        //Again send discovery broadcast message
        discovery_service.message_interface->send_discovery();
    }
    else
        discovery_service.timer->stop_timer();

}


void start_discovery(){

    if(discovery_service.message_interface!=NULL && discovery_service.timer!=NULL ){
        //start discovery
        discovery_service.message_interface->send_discovery();
        //start timer
        ESP_LOGI(TAG,"timer interval %"PRIu32,discovery_service.discovery_interval);
        discovery_service.timer->start_timer(discovery_service.discovery_interval);
        //Get current time and save it in the state
        discovery_service.discovery_start_time=discovery_service.timer->get_current_time();
    
    }

}







discovery_service_interface_t* discovery_service_init(config_espnow_discovery* config){


    if(config==NULL)
        return NULL;

    discovery_service.message_interface=config->discovery;
    

    discovery_service.whitelist=config->whitelist;
    discovery_service.timer=config->timer;
    discovery_service.discovery_duration=config->discovery_duration;
    discovery_service.discovery_interval=config->discovery_interval;
    
    

    //Right now it is only invokd once. in future the button interface will be useed
    //start_discovery();
    //Now assigning the handlers pointers to the handers
    discovery_service.discovery_callback_handlers.comm_callback_handler.process_discovery_acknowledgement_callback=discovery_acknowledgement_handler;
    discovery_service.discovery_callback_handlers.comm_callback_handler.process_discovery_callback=incoming_discovery_handler;
    discovery_service.discovery_callback_handlers.input_callback_handler.button_event_callback=NULL;  //not yet used
    discovery_service.discovery_callback_handlers.timer_callback_handler.timer_handler=timer_elapsed_handler;

    return &discovery_service.discovery_callback_handlers;


}
