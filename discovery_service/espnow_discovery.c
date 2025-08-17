
#include "espnow_discovery.h"


#define         DISCOVERY_DURATION          5000    //ms

static const char* TAG= "discovery";


static struct{
    //Total duration to run the discovery boradcast
    uint32_t discovery_duration;            //milliseconds
    ///Interval between each discoovery broadcast. Must be less than discovery_duration
    uint32_t discovery_interval;             //milliseconds
    uint32_t current_time;
    espnow_discovery_interface_t* discovery;
    discovery_whitelist_interface_t* whitelist;
    discovery_timer_interface_t* timer;
}discovery_service={0};




/// @brief Will be invoked when there is some incoming discovery packet
/// Checks if sender is in whitelist and adds as peer, ignores otherwise
/// @param mac_addr 
static void incoming_discovery_handler(const uint8_t *mac_addr){


    //Check if device is in the white list
    if(discovery_service.whitelist->is_white_listed(mac_addr)==true){
        //if yes then add as peer
        discovery_service.discovery->add_peer(mac_addr);
        //And tell the device that its discoveery was  received, so that it stops if desires
        discovery_service.discovery->acknowledge_the_discovery(mac_addr);
    }

}


/// @brief Invoked when an acknowledgment message of discovery arrives
/// @param mac_addr 
/// @return 
static void discovery_acknowledgement_handler(const uint8_t *mac_addr){
    if(discovery_service.whitelist->is_white_listed(mac_addr)==true){
            //if yes then add as peer
            discovery_service.discovery->add_peer(mac_addr);
    }
    

    

}


/// @brief Invoked after discovery interval
static  void timer_elapsed_handler(){

    //Stop the discovery
    uint32_t current_time=discovery_service.timer->get_current_time();
    uint32_t previous_time=discovery_service.current_time;
    uint32_t duration=discovery_service.discovery_duration;
    //If time passed is less than the discovery duration than 
    if(current_time-previous_time<duration){
        //Again send discovery broadcast message
        discovery_service.discovery->send_discovery();
    }
    else
        discovery_service.timer->stop_timer();

}


void start_discovery(){

    if(discovery_service.discovery!=NULL && discovery_service.timer!=NULL ){
        //start discovery
        discovery_service.discovery->send_discovery();
        //start timer
        discovery_service.timer->start_timer(discovery_service.discovery_interval);
        //Get current time and save it in the state
        discovery_service.current_time=discovery_service.timer->get_current_time();
    
    }

}







esp_err_t discovery_service_init(config_espnow_discovery* config){


    if(config==NULL)
        return ESP_FAIL;

    discovery_service.discovery=config->discovery;
    discovery_service.discovery->process_discovery_acknowledgement_callback=discovery_acknowledgement_handler;
    discovery_service.discovery->process_discovery_callback=incoming_discovery_handler;

    discovery_service.whitelist=config->whitelist;
    discovery_service.timer=config->timer;
    discovery_service.timer->discovery_timer_callback=timer_elapsed_handler;
    discovery_service.discovery_duration=config->discovery_duration;
    discovery_service.discovery_interval=config->discovery_interval;
    

    //Right now it is only invokd once. in future the button interface will be useed
    //start_discovery();
    
    return ESP_OK;


}
