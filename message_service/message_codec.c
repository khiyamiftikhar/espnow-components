
#include "stdio.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "message_codec.h"
#include "esp_log.h"



#define         QUEUE_MAX_ELEMENTS          5

static const char* TAG="message-decode";
DEFINE_EVENT_ADAPTER(MESSAGE_CODEC);




typedef enum{
    MSG_TYPE_COMMAD,
    MSG_TYPE_STATUS
}message_codec_message_type_t;



typedef struct {
    message_codec_message_type_t msg_type;
    message_codec_command_type_t cmd;
    message_codec_lock_status_t lock_status;
    void* context;

}message_codec_message_t;


typedef enum{

    MSG_SENT=0,
    MSG_RECEIVED
}msg_info_type_t;

typedef struct{
    msg_info_type_t type;
    uint8_t mac[6];
    message_codec_message_t msg;
    bool success;
}callback_queue_data_t;





DEFINE_EVENT_ADAPTER(MESSAGE_DECODER);

static struct {
    esp_now_transport_msg_interface_t* msg_interface;
    database_interface_t* database_interface;
    TaskHandle_t callback_handler_task;
    QueueHandle_t callback_handler_queue;
    QueueHandle_t context_queue;            //To put and get contexts in order
    
}message_codec_state={0};






esp_err_t message_codec_send_command(uint8_t* mac_addr,message_codec_command_type_t command,void* context){



    message_codec_message_t response_msg = {
        .msg_type = MSG_TYPE_COMMAD,
        .cmd = command,  // Original command that triggered this response
    };
    
    BaseType_t err=xQueueSend(message_codec_state.context_queue,&context,0);

    //For example if queue is  full
    if(err!=pdTRUE)
        return ESP_FAIL;

    esp_err_t result = message_codec_state.msg_interface->esp_now_transport_send_data(mac_addr, (uint8_t *)&response_msg, sizeof(response_msg));


    
    if (result == ESP_OK) {
        ESP_LOGI(TAG, "Status response sent successfully: %d", result);
    } else {
        ESP_LOGE(TAG, "Failed to send status response: %s", esp_err_to_name(result));
    }

    return result;


}


/**
 * Send status response via ESP-NOW
 */
esp_err_t message_codec_send_status(uint8_t* mac_addr,message_codec_lock_status_t status){

    message_codec_message_t response_msg = {
        .msg_type = MSG_TYPE_STATUS,
        .cmd = MESSAGE_COMMAND_LOCK_STATUS,  // Original command that triggered this response
        .lock_status = status
    };
    
    esp_err_t result = message_codec_state.msg_interface->esp_now_transport_send_data(mac_addr, (uint8_t *)&response_msg, sizeof(response_msg));
    
    if (result == ESP_OK) {
        ESP_LOGI(TAG, "Status response sent successfully: %d", status);
    } else {
        ESP_LOGE(TAG, "Failed to send status response: %s", esp_err_to_name(result));
    }

    return result;
}


/**
 * Handle incoming command messages
 */

static void status_msg_handler(const uint8_t* mac_addr,message_codec_lock_status_t cmd){
//static void handle_command_message(const uint8_t *mac_addr, const message_codec_message_t *msg) {
    
    MESSAGE_CODEC_post_event(MESSAGE_SERVICE_ROUTINE_EVENT_GATE_STATUS_ARRIVED,(void*)&cmd,sizeof(message_codec_lock_status_t));
}
            
            
    



/**
 * Handle incoming command messages
 */

static void command_msg_handler(const uint8_t* mac_addr,message_codec_command_type_t cmd){
//static void handle_command_message(const uint8_t *mac_addr, const message_codec_message_t *msg) {
    switch (cmd) {
        case MESSAGE_COMMAND_OPEN_LOCK:

            ESP_LOGI(TAG, "Command: Open lock");
            MESSAGE_CODEC_post_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_OPEN_GATE,NULL,0);
            
            
            break;
            
        case MESSAGE_COMMAND_CLOSE_LOCK:
            ESP_LOGI(TAG, "Command: Close lock");
            MESSAGE_CODEC_post_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_CLOSE_GATE,NULL,0);
            
            break;
            
        case MESSAGE_COMMAND_LOCK_STATUS:
            MESSAGE_CODEC_post_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_SEND_GATE_STATUS,(void*)mac_addr,sizeof(uint8_t)*6);

            //Get  lock status
            //send lock status
            //lock_system_lock_status_t lock_status=gate_node.lock->get_lock_status();
            //send_status_response(mac_addr,lock_status);
            
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown command: %d", cmd);
            break;
    }
}


/**
 * Main message processing function
 * Call this from your ESP-NOW receive callback
 */
static void message_received_handler(const uint8_t *mac_addr, const message_codec_message_t *msg) {
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
            status_msg_handler(mac_addr,msg->lock_status);
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown message type: %d", msg->msg_type);
            break;
    }
}


static void message_sent_handler(uint8_t* mac,bool success){


    void* context=NULL;
    message_send_ack_t send_ack;
    //Retreive in order

    ESP_LOGI(TAG,"send success after task %d",success);
    xQueueReceive(message_codec_state.context_queue,&context,0);
    send_ack.context=context;
    send_ack.success=success;

    MESSAGE_CODEC_post_event(MESSAGE_SERVICE_ROUTINE_EVENT_SEND_STATUS,&send_ack,sizeof(send_ack));

    
    //ESP_LOGI(TAG,",message sent callback posted");
}




/// @brief It will handle the updates pushed to queue y the callbacks invoked by the esp-now-comm
/// @param args 
static void message_callbacks_handler_task(void* args){

    callback_queue_data_t queue_data={0};

    while(1){


        if(xQueueReceive(message_codec_state.callback_handler_queue,&queue_data,portMAX_DELAY)==pdTRUE)

            switch(queue_data.type){
                case MSG_RECEIVED:
                    message_received_handler(queue_data.mac,&queue_data.msg);

                    break; 
                case MSG_SENT:
                    message_sent_handler(queue_data.mac,queue_data.success);
                    
                    break;
                default:
                    break;

            }
    }

}


/// @brief It will be invoked by the esp-now-comm component
/// @param src_mac 


static void message_recevied_callback_handler(const uint8_t* mac,const uint8_t* msg, size_t len ){

    callback_queue_data_t queue_data;

    queue_data.type=MSG_RECEIVED;
    if(len!=sizeof(message_codec_message_t))
        return;

    message_codec_message_t* lock_msg=(message_codec_message_t*)msg;
    memcpy(queue_data.mac,mac,sizeof(queue_data.mac));
    memcpy(&queue_data.msg,lock_msg,len);
    xQueueSendFromISR(message_codec_state.callback_handler_queue,&queue_data,NULL);


}

/// @brief It will be invoked by the esp-now-comm component
/// @param src_mac 
static void message_sent_callback_handler(const uint8_t *mac_addr, bool success){

    callback_queue_data_t queue_data;

    queue_data.type=MSG_SENT;
    memcpy(queue_data.mac,mac_addr,sizeof(queue_data.mac));
    ESP_LOGI(TAG,"send success in cb %d",success);
    queue_data.success=success;
    xQueueSendFromISR(message_codec_state.callback_handler_queue,&queue_data,NULL);


}




esp_err_t message_codec_init(message_codec_config_t* config){

    if(config==NULL)
        return ESP_FAIL;
    
    message_codec_state.msg_interface=config->msg_interface;
    message_codec_state.database_interface=config->database_interface;

    //Register the callbacks that will be invoked by the esp-now-comm component
    message_codec_state.msg_interface->set_esp_now_device_data_rcv_cb(message_recevied_callback_handler);
    message_codec_state.msg_interface->set_esp_now_device_data_sent_cb(message_sent_callback_handler);

    message_codec_state.callback_handler_queue=xQueueCreate(QUEUE_MAX_ELEMENTS,sizeof(callback_queue_data_t));

    ESP_ERROR_CHECK(message_codec_state.callback_handler_queue==NULL);
    
    BaseType_t ret=xTaskCreate(message_callbacks_handler_task,"message task",4096,NULL,5,&message_codec_state.callback_handler_task);

    ESP_ERROR_CHECK(ret!=1);

    message_codec_state.context_queue = xQueueCreate(10, sizeof(void*));   
    

    //ESP_LOGI(TAG,"check in gate service init %d",gate_node.list->is_in_whitelist(NULL));


    MESSAGE_CODEC_register_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_OPEN_GATE,NULL,NULL);
    MESSAGE_CODEC_register_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_CLOSE_GATE,NULL,NULL);
    MESSAGE_CODEC_register_event(MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_SEND_GATE_STATUS,NULL,NULL);
    MESSAGE_CODEC_register_event(MESSAGE_SERVICE_ROUTINE_EVENT_GATE_STATUS_ARRIVED,NULL,NULL);
    MESSAGE_CODEC_register_event(MESSAGE_SERVICE_ROUTINE_EVENT_SEND_STATUS,NULL,NULL);
    

    


    //The msg sent call back handler does not make sense at the gate node
    //because the gate node does not require confirmation 

    return ESP_OK;
}