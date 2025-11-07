
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "message_codec.h"
#include "esp_log.h"



#define         QUEUE_MAX_ELEMENTS          5

static const char* TAG="message-decode";
DEFINE_EVENT_ADAPTER(MESSAGE_CODEC);

typedef enum{
    LOCK_STATUS_OPEN,
    LOCK_STATUS_CLOSED,
    LOCK_STATUS_OPENING,
    LOCK_STATUS_CLOSING,
    LOCK_STATUS_CLOSED_IDLE,    //These new states added for automotive lock actuator
    LOCK_STATUS_OPENDED_IDLE,

}lock_system_lock_status_t;



typedef enum{
    MSG_TYPE_COMMAD,
    MSG_TYPE_STATUS
}lock_system_message_type_t;

typedef enum{
    LOCK_SYSTEM_COMMAND_OPEN_LOCK,
    LOCK_SYSTEM_COMMAND_CLOSE_LOCK,
    LOCK_SYSTEM_COMMAND_LOCK_STATUS
}lock_system_command_type_t;


typedef struct {
    lock_system_message_type_t msg_type;
    lock_system_command_type_t cmd;
    lock_system_lock_status_t lock_status;
    void* context;

}lock_system_message_t;


typedef enum{

    MSG_SENT=0,
    MSG_RECEIVED
}msg_info_type_t;

typedef struct{
    msg_info_type_t type;
    uint8_t mac[6];
    lock_system_message_t msg;
    bool success;
}callback_queue_data_t;





DEFINE_EVENT_ADAPTER(MESSAGE_DECODER);

static struct {
    esp_now_transport_msg_interface_t* msg_interface;
    database_interface_t* database_interface;
    TaskHandle_t callback_handler_task;
    QueueHandle_t callback_handler_queue;
    
}message_codec_state={0};







/**
 * Send status response via ESP-NOW
 */
static void status_msg_handler(lock_system_lock_status_t status) {
    lock_system_message_t response_msg = {
        .msg_type = MSG_TYPE_STATUS,
        .cmd = LOCK_SYSTEM_COMMAND_LOCK_STATUS,  // Original command that triggered this response
        .lock_status = status
    };
    
    esp_err_t result = gate_node.msg->send_msg(mac_addr, (uint8_t *)&response_msg, sizeof(response_msg));
    
    if (result == ESP_OK) {
        ESP_LOGI(TAG, "Status response sent successfully: %d", status);
    } else {
        ESP_LOGE(TAG, "Failed to send status response: %s", esp_err_to_name(result));
    }
}



/**
 * Handle incoming command messages
 */


static void command_msg_handler(uin8_t* mac_addr,lock_system_command_type_t cmd){
//static void handle_command_message(const uint8_t *mac_addr, const lock_system_message_t *msg) {
    switch (cmd) {
        case LOCK_SYSTEM_COMMAND_OPEN_LOCK:

            ESP_LOGI(TAG, "Command: Open lock");
            MESSAGE_CODEC_post_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_OPEN_GATE,NULL,0);
            
            
            break;
            
        case LOCK_SYSTEM_COMMAND_CLOSE_LOCK:
            ESP_LOGI(TAG, "Command: Close lock");
            MESSAGE_CODEC_post_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_CLOSE_GATE,NULL,0);
            
            break;
            
        case LOCK_SYSTEM_COMMAND_LOCK_STATUS:
            MESSAGE_CODEC_post_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_SEND_GATE_STATUS,NULL,0);

            //Get  lock status
            //send lock status
            //lock_system_lock_status_t lock_status=gate_node.lock->get_lock_status();
            //send_status_response(mac_addr,lock_status);
            
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown command: %d", msg->cmd);
            break;
    }
}


/**
 * Main message processing function
 * Call this from your ESP-NOW receive callback
 */
static void message_received_handler(const uint8_t *mac_addr, const lock_system_message_t *msg) {
    ESP_LOGI(TAG,"process received");
    if (!msg || !mac_addr) {
        ESP_LOGE(TAG, "Invalid parameters");
        return;
    }


    ESP_LOGI(TAG, "Received message - Type: %d, Command: %d, Status: %d", 
             msg->msg_type, msg->cmd, msg->lock_status);

    switch (msg->msg_type) {
        case MSG_TYPE_COMMAD:  // Command message

            command_msg_handler(mac_addr, msg->cmd);
            break;
            
        case MSG_TYPE_STATUS:  // Status message
            status_msg_handler(msg->lock_status);
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown message type: %d", msg->msg_type);
            break;
    }
}






/// @brief It will handle the updates pushed to queue y the callbacks invoked by the esp-now-comm
/// @param args 
static void discovery_callbacks_handler_task(void* args){

    callback_queue_data_t queue_data={0};

    while(1){


        if(xQueueReceive(discovery_service.callback_queue,&queue_data,portMAX_DELAY)==pdTRUE)

            switch(queue_data.type){
                case MSG_RECEIVED:
                    message_received_handler(queue_data.mac,queue_data.msg);

                    break; 
                case MSG_SENT:
                    msg_sent_handler(queue_data.success);
                    
                    break;
                default:
                    break;

            }
    }

}


/// @brief It will be invoked by the esp-now-comm component
/// @param src_mac 


static void message_recevied_callback_handler(const uint8_t* mac,const uint8_t* msg, size_t length ){

    callback_queue_data_t queue_data;

    queue_data.type=MSG_RECEIVED;
    if(len!=sizeof(lock_system_message_t))
        return;

    lock_system_message_t* lock_msg=(lock_system_message_t*)msg;
    memcpy(queue_data.mac,mac,sizeof(queue_data.mac));
    memcpy(queue_data.msg,lock_msg,length);
    xQueueSendFromISR(message_codec_state.callback_handler_queue,&queue_data,NULL);


}

/// @brief It will be invoked by the esp-now-comm component
/// @param src_mac 
static void message_sent_callback_handler(const uint8_t *mac_addr, bool success){

    callback_queue_data_t queue_data;

    queue_data.type=MSG_SENT;
    memcpy(queue_data.mac,src_mac,sizeof(queue_data.mac));
    queue_data.success-

    xQueueSendFromISR(discovery_service.callback_queue,&queue_data,NULL);


}




esp_err_t lock_system_message_codec_init(message_codec_config_t* config){

    if(config==NULL)
        return ESP_FAIL;
    
    message_codec_state.msg_interface=config->msg_interface;
    message_codec_state.database_interface=config->database_interface;
    
    BaseType_t ret=xTaskCreate(message_callback_handler_task,"message task",configMINIMAL_STACK_SIZE,NULL,5,&message_codec_state.callback_handler_task);

    ESP_ERROR_CHECK(ret!=1);

    
    message_codec_state.callback_handler_queue=xQueueCreate(QUEUE_MAX_ELEMENTS,sizeof(callback_queue_data_t));

    ESP_ERROR_CHECK(message_codec_state.callback_handler_queue==NULL);

    //ESP_LOGI(TAG,"check in gate service init %d",gate_node.list->is_in_whitelist(NULL));


    MESSAGE_CODEC_register_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_OPEN_GATE,NULL,NULL);
    MESSAGE_CODEC_register_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_CLOSE_GATE,NULL,NULL);
    MESSAGE_CODEC_register_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_SEND_GATE_STATUS,NULL,NULL);
    MESSAGE_CODEC_register_event(MESSAGE_SERVICE_ROUTINE_EVENT_GATE_STATUS_ARRIVED,NULL,NULL);
    

    


    //The msg sent call back handler does not make sense at the gate node
    //because the gate node does not require confirmation 

    return ESP_OK;
}