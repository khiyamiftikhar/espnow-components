#include "string.h"
#include "home_node.h"
#include "message_def.h" 
#include "esp_log.h"

static const char* TAG="home node";

static const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};


static struct{
    
    //when the user sends command , and when it is desired to send user the status of lock
    
    //The common interface and data types between the gate node and home node, to send and recive data using espnow
    node_msg_interface_t* msg_interface;
    //check whether the sender is whitelisted
    node_white_list_interface_t* white_list;
    //GEt the mac address of the gate node
    gate_node_id_interface_t* gate_node;

    

    //user_command_callback callback;

}home_node_service={0};


static esp_err_t send_command(const uint8_t *mac_addr, lock_system_message_type_t msg_type,lock_system_command_type_t cmd) {


    lock_system_message_t cmd_msg = {
        .msg_type = msg_type,
        .cmd = cmd,
        .lock_status = 0        //Not used
    };

    esp_err_t ret=0;
    ret= home_node_service.msg_interface->send_msg(mac_addr,(uint8_t*)&cmd_msg,sizeof(cmd_msg));

    ESP_LOGI(TAG,"sent command success %d",ret);
    return ret;

}


esp_err_t home_node_send_command(user_command_t cmd){
    
    uint8_t gate_node_mac_addr[6];
    home_node_service.gate_node->get_gate_node_mac(gate_node_mac_addr);
    esp_err_t ret;
    switch (cmd){
        case USER_COMMAND_LOCK_OPEN:
            ret=send_command(gate_node_mac_addr,MSG_TYPE_COMMAD,LOCK_SYSTEM_COMMAND_OPEN_LOCK);
            break;


        case USER_COMMAND_LOCK_CLOSE:
            ret=send_command(gate_node_mac_addr,MSG_TYPE_COMMAD,LOCK_SYSTEM_COMMAND_CLOSE_LOCK);
            break;

        case USER_COMMAND_LOCK_STATUS:
            ret=send_command(gate_node_mac_addr,MSG_TYPE_COMMAD,LOCK_SYSTEM_COMMAND_LOCK_STATUS);
            break;
        default:
            ret=send_command(gate_node_mac_addr,MSG_TYPE_COMMAD,LOCK_SYSTEM_COMMAND_LOCK_STATUS);
            break;

    }

    return ret;

}



static void msg_received_handler(const uint8_t *mac_addr, const uint8_t *data, size_t len){

    
    if(data==NULL || mac_addr==NULL || len!=sizeof(lock_system_message_t))
        return;
    
    lock_system_message_t* msg=(lock_system_message_t*)data;
    
    ///Only status message can arrive at the home node
    if(msg->msg_type!=MSG_TYPE_STATUS)
        return;
    
    lock_status_t lock_status=msg->lock_status;

    //Inform the user about current status of the lock
    //home_node_service.user_interaction->inform_lock_status(lock_status);
}

static void msg_sent_handler(const uint8_t *mac_addr, bool success){

    ESP_LOGI(TAG,"hello came in send handler %d",success);
    
    //Only inform about the send status if it is not a broadcast
    if(memcmp(mac_addr,BROADCAST_MAC,sizeof(BROADCAST_MAC))!=0){
      //  home_node_service.user_interaction->inform_command_status(success);
    }

}

esp_err_t home_node_service_create(home_node_config_t* config){

    if(config==NULL)
        return ESP_FAIL;
    home_node_service.user_interaction=config->user_output;
    
    
    home_node_service.msg_interface=config->msg_interface;
    
    
    home_node_service.gate_node=config->gate_node_id;
    home_node_service.white_list=config->white_list;

    
    
    
    

    
    return ESP_OK;
}
