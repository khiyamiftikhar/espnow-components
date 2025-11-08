

#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "discovery_timer.h"
#include "espnow_discovery.h"
#include "inttypes.h"
#include "event_system_adapter.h"


#define         DISCOVERY_DURATION          5000    //ms
#define         DISCOVERY_MAX_DEVICES       5
#define         DISCOVERY_INTERVAL          500     //ms
#define         QUEUE_MAX_ELEMENTS          5
static const char* TAG= "discovery";

typedef enum{
    DISCOVERY_INCOMING_EVENT=0,
    DISCOVERY_ACK_EVENT,
}discovery_event_t;

typedef struct{
    uint8_t mac[6];
    discovery_event_t event;
}callback_queue_data_t;

typedef struct {
    uint8_t mac[6];
}discovery_device_t;

typedef struct {
    discovery_device_t devices[DISCOVERY_MAX_DEVICES];
    uint8_t result_count;
}discovery_result_t;


static struct{
    //Total duration to run the discovery boradcast
    uint32_t discovery_duration;            //milliseconds
    ///Interval between each discoovery broadcast. Must be less than discovery_duration
    uint32_t discovery_interval;             //milliseconds
    uint32_t discovery_start_time;
    TaskHandle_t discovery_task;
    TaskHandle_t callback_handler_task;     //The task that decouples lower level espnow-comm component
    QueueHandle_t callback_queue;
    //These are the interfaces that it uses and are injected to it
    esp_now_transport_discovery_interface_t* discovery_interface;
    esp_now_peer_manager_interface_t* peer_manager_interface;
    database_interface_t* database_interface;
    discovery_timer_interface_t* timer;
    //These are the callbacks that are assigned internally and externally to appropriate invokers
    discovery_result_t discovery_result;
    bool discovery_state;
}discovery_service={0};


//This macro defined in the event_system_adapter_object creates custom apis with this name appended
DEFINE_EVENT_ADAPTER(DISCOVERY_SERVICE);


/// @brief Will be invoked when there is some incoming discovery packet
/// Checks if sender is in whitelist and adds as peer, ignores otherwise
/// @param mac_addr 
static void incoming_discovery_handler(const uint8_t *mac_addr){


    //Check if device is in the white list
    if(discovery_service.database_interface->is_white_listed(mac_addr)==true){
        //if yes then add as peer
        discovery_service.peer_manager_interface->esp_now_transport_add_peer(mac_addr);
        //And tell the device that its discoveery was  received, so that it stops if desires
        discovery_service.discovery_interface->esp_now_transport_send_discovery_ack(mac_addr);
    }

    //if(discovery_service.message_interface->is_peer_exist(mac_addr)!=true)
      //  discovery_service.message_interface.

}


/// @brief Invoked when an acknowledgment message of discovery arrives
/// @param mac_addr 
/// @return 
static void discovery_acknowledgement_handler(const uint8_t *mac_addr){

    memcpy(discovery_service.discovery_result.devices->mac,mac_addr,6);
    discovery_service.discovery_result.result_count++;
    if(discovery_service.database_interface->is_white_listed(mac_addr)==true){
            ESP_LOGI(TAG,"yess added");
            discovery_service.peer_manager_interface->esp_now_transport_add_peer(mac_addr);
    }


}



/// @brief It will handle the updates pushed to queue y the callbacks invoked by the esp-now-comm
/// @param args 
static void discovery_callbacks_handler_task(void* args){

    callback_queue_data_t queue_data={0};

    while(1){


        if(xQueueReceive(discovery_service.callback_queue,&queue_data,portMAX_DELAY)==pdTRUE)

            switch(queue_data.event){
                case DISCOVERY_INCOMING_EVENT:
                    incoming_discovery_handler(queue_data.mac);

                    break; 
                case DISCOVERY_ACK_EVENT:
                    discovery_acknowledgement_handler(queue_data.mac);
                    
                    break;

            }
    }

}


/// @brief It will be invoked by the esp-now-comm component
/// @param src_mac 

static void discovery_callback_handler(const uint8_t* src_mac){

    callback_queue_data_t queue_data;

    memcpy(queue_data.mac,src_mac,sizeof(queue_data.mac));
    queue_data.event=DISCOVERY_INCOMING_EVENT;

    xQueueSendFromISR(discovery_service.callback_queue,&queue_data,NULL);


}

/// @brief It will be invoked by the esp-now-comm component
/// @param src_mac 
static void discovery_ack_callback_handler(const uint8_t* src_mac){

    callback_queue_data_t queue_data;

    memcpy(queue_data.mac,src_mac,sizeof(queue_data.mac));
    queue_data.event=DISCOVERY_ACK_EVENT;

    xQueueSendFromISR(discovery_service.callback_queue,&queue_data,NULL);


}




static void stop_discovery(){
    ESP_LOGI(TAG,"stopping discovery");
    discovery_service.timer->stop_timer();
    //Must be guarded with a mutex lock
    discovery_service.discovery_state=false;

}

/// @brief Invoked after discovery interval
static  void timer_elapsed_handler(){

    //Stop the discovery
    uint32_t current_time=discovery_service.timer->get_current_time();
    uint32_t discovery_start_time=discovery_service.discovery_start_time;
    uint32_t duration=discovery_service.discovery_duration;
    //If time passed is less than the discovery duration than 
    //ESP_LOGI(TAG, "current time %"PRIu32" previous time %"PRIu32 "current duration%"PRIu32 "total duration%"PRIu32, current_time, discovery_start_time,(current_time-discovery_start_time),duration); 

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t notify_result;
    
    //uint32_t result=total_devices_found;

    if((current_time-discovery_start_time)<duration){
    
        notify_result = xTaskNotifyFromISR(discovery_service.discovery_task, 1, 
                                          eSetValueWithOverwrite, 
                                          &xHigherPriorityTaskWoken);
    }

    else{
        notify_result = xTaskNotifyFromISR(discovery_service.discovery_task, 0, 
                                          eSetValueWithOverwrite, 
                                          &xHigherPriorityTaskWoken);
        }

    
    /*
    if (notify_result != pdPASS) {
        // Notification failed - set a backup flag
        process_failed_flag = true;
        backup_result = result;
    }
    */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
       
}

/// @brief Just Starts the timer and sets the current time in the state variable, 
///         So that discovey can be run for a set time duration

void start_discovery(){


    //Set the record to 0

    //Allready running
    ESP_LOGI(TAG,"starting discovery");
    if(discovery_service.discovery_state==true)
        return;

    memset(&discovery_service.discovery_result,0,sizeof(discovery_result_t));
    
    if(discovery_service.discovery_interface!=NULL && discovery_service.timer!=NULL ){
        //start discovery
        //discovery_service.message_interface->send_discovery();
        //start timer
        ESP_LOGI(TAG,"timer interval %"PRIu32,discovery_service.discovery_interval);
        discovery_service.timer->start_timer(discovery_service.discovery_interval);
        //Get current time and save it in the state
        discovery_service.discovery_start_time=discovery_service.timer->get_current_time();
    
    }
    discovery_service.discovery_state=true;
}

/// @brief Return the total number of devices discovered. This variable set to 0 on start discovery
/// @return 
esp_err_t discovered_devices_count(){
    return discovery_service.discovery_result.result_count;
}

/// @brief //If task notify comes with 1 , then send discovery, else stop discovery
/// @param args 


static void discovery_task(void* args){

    uint32_t  keep_alive=0;
    //If task notify comes with 1 , then send discovery
    //else stop discovery
    while(1){
         if(xTaskNotifyWait(0, 0, &keep_alive, portMAX_DELAY)==pdTRUE){
            if(keep_alive==1)
                discovery_service.discovery_interface->esp_now_transport_send_discovery();
            else{
                stop_discovery();
                
                //if(discovery_service.message_interface->process_discovery_completion_callback!=NULL){
                    ESP_LOGI(TAG,"discovery over");
                    DISCOVERY_SERVICE_post_event(DISCOVERY_EVENT_DISCOVERY_COMPLETE,(void*)&discovery_service.discovery_result.result_count,sizeof(discovery_service.discovery_result.result_count));
                  //  discovery_service.message_interface->process_discovery_completion_callback(discovery_service.discovery_result.result_count);
                //}
            }
        }
    }

}



esp_err_t discovery_service_init(config_espnow_discovery* config){


    if(config==NULL)
        return ESP_FAIL;


    discovery_timer_implementation_t* timer_interface=timer_create(config->discovery_interval);

    discovery_service.timer=&timer_interface->methods;
    timer_interface->callback_handler=timer_elapsed_handler;


    BaseType_t ret=xTaskCreate(discovery_task,"discovery task",4096,NULL,5,&discovery_service.discovery_task);

    ESP_ERROR_CHECK(ret!=1);

    ret=xTaskCreate(discovery_callbacks_handler_task,"callback handler task",4096,NULL,5,&discovery_service.callback_handler_task);
    ESP_ERROR_CHECK(ret!=1);

    discovery_service.callback_queue=xQueueCreate(QUEUE_MAX_ELEMENTS,sizeof(callback_queue_data_t));

    ESP_ERROR_CHECK(discovery_service.callback_queue==NULL);

    //Register the discovery completion
    DISCOVERY_SERVICE_register_event(DISCOVERY_EVENT_DISCOVERY_COMPLETE,NULL,NULL);

    discovery_service.discovery_interface=config->discovery_interface;
    


    //Register the callbacks
    discovery_service.discovery_interface->set_esp_now_device_discovery_cb(discovery_callback_handler);
    discovery_service.discovery_interface->set_esp_now_device_discovery_ack_cb(discovery_ack_callback_handler);

    discovery_service.peer_manager_interface=config->peer_manager_interface;
    discovery_service.database_interface=config->database_interface;
    discovery_service.discovery_duration=config->discovery_duration;
    discovery_service.discovery_interval=config->discovery_interval;

    return ESP_OK;

}
