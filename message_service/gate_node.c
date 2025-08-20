#include "stdio.h"
#include "message_def.h"
#include "gate_node.h"
#include "esp_log.h"



static const char* TAG="gate-node";


static struct {
    node_msg_interface_t* msg;
    node_white_list_interface_t* list;
    gate_node_lock_interface_t* lock;
    gate_node_callback_handlers_t callback_handlers;
}gate_node={0};







/**
 * Send status response via ESP-NOW
 */
static void send_status_response(const uint8_t *mac_addr, lock_system_lock_status_t status) {
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
static void handle_command_message(const uint8_t *mac_addr, const lock_system_message_t *msg) {
    switch (msg->cmd) {
        case LOCK_SYSTEM_COMMAND_OPEN_LOCK:
            ESP_LOGI(TAG, "Command: Open lock");
            gate_node.lock->set_lock_open();
            // Send immediate response with current status
            
            break;
            
        case LOCK_SYSTEM_COMMAND_CLOSE_LOCK:
            ESP_LOGI(TAG, "Command: Close lock");
            gate_node.lock->set_lock_close();
            // Send immediate response with current status
            
            break;
            
        case LOCK_SYSTEM_COMMAND_LOCK_STATUS:
            ESP_LOGI(TAG, "Command: Status request");

            //Get  lock status
            //send lock status
            lock_system_lock_status_t lock_status=gate_node.lock->get_lock_status();
            send_status_response(mac_addr,lock_status);
            
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
static void process_lock_message(const uint8_t *mac_addr, const lock_system_message_t *msg) {
    ESP_LOGI(TAG,"process received");
    if (!msg || !mac_addr) {
        ESP_LOGE(TAG, "Invalid parameters");
        return;
    }


    ESP_LOGI(TAG, "Received message - Type: %d, Command: %d, Status: %d", 
             msg->msg_type, msg->cmd, msg->lock_status);

    switch (msg->msg_type) {
        case MSG_TYPE_COMMAD:  // Command message
            handle_command_message(mac_addr, msg);
            break;
            
        case MSG_TYPE_STATUS:  // Status message
            //handle_status_message(msg);
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown message type: %d", msg->msg_type);
            break;
    }
}



static void msg_received_handler(const uint8_t* mac,const uint8_t* msg, size_t length ){
    
    //ESP_LOGI(TAG,"in received");
    if(msg==NULL || mac==NULL)
        return;
    //ESP_LOGI(TAG,"before list check");
    if(gate_node.list==NULL){
        ESP_LOGI(TAG,"no list");
        return;
    }
    //ESP_LOGI(TAG,"before list method check");
    if(gate_node.list->is_in_whitelist==NULL){
        ESP_LOGI(TAG,"no list method");
        return;
    }
    //If the node is not a peer then
    //ESP_LOGI(TAG,"before mac check");
    if(gate_node.list->is_in_whitelist(mac)!=true){
        ESP_LOGI(TAG,"not in list");
        return;
    }
    //ESP_LOGI(TAG,"after mac check");
    if (length == sizeof(lock_system_message_t)) {
        lock_system_message_t *message = (lock_system_message_t*) msg;
        process_lock_message(mac,message);

    }

}


gate_node_service_interface_t* gate_node_init(gate_node_config_t* config){

    if(config==NULL)
        return NULL;
    
    gate_node.list=config->list;
    gate_node.lock=config->lock;
    gate_node.msg=config->msg;
    
    ESP_LOGI(TAG,"check in gate service init %d",gate_node.list->is_in_whitelist(NULL));
    gate_node.callback_handlers.msg_received_handler=msg_received_handler;


    //The msg sent call back handler does not make sense at the gate node
    //because the gate node does not require confirmation 

    return &gate_node.callback_handlers;
}