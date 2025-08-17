#include <stdio.h>
#include "gate_node.h"
#include "unity.h"
#include "esp_log.h"
#include "esp_err.h"
#include <inttypes.h>
#include "message_def.h"
static const char* TAG="test gate node";


static gate_node_config_t config;
static node_msg_interface_t msg;
static node_white_list_interface_t list;
static gate_node_lock_interface_t lock;




static esp_err_t send_message(const uint8_t *mac_addr, const uint8_t *data, size_t len){

    ESP_LOGI(TAG,"sending message");
    return 0;
}

static bool is_in_whitelist(const uint8_t *mac_addr){

    return true;
}
static lock_system_lock_status_t gate=false;

static esp_err_t set_lock_open(){
    gate=LOCK_STATUS_OPEN;
    ESP_LOGI(TAG,"lock %d",gate);
    return 0;
}

static esp_err_t set_lock_close(){
    gate=LOCK_STATUS_CLOSED;
    ESP_LOGI(TAG,"lock %d",gate);
    return 0;
}

static lock_system_lock_status_t get_lock_status(){
    
    ESP_LOGI(TAG,"lock %d",gate);
    return gate;
}


static void gateSetUp(){
    
    msg.send_msg=send_message;
    list.is_in_whitelist=is_in_whitelist;
    lock.set_lock_close=set_lock_close;
    lock.set_lock_open=set_lock_open;
    lock.get_lock_status=get_lock_status;
    list.is_in_whitelist=is_in_whitelist;
    config.msg=&msg;
    config.list=&list;
    config.lock=&lock;
    
    gate_node_init(&config);

}


TEST_CASE("Gate node","[Unit Test: LOCK close]"){

    uint8_t mac[]={1,2,3,4,5,6};
    gateSetUp();
    lock_system_message_t message={0};
    message.msg_type=MSG_TYPE_COMMAD;
    message.cmd=LOCK_SYSTEM_COMMAND_CLOSE_LOCK;
    
    msg.msgReceivedCallback(mac,(const uint8_t*)&message,sizeof(lock_system_message_t));

}

TEST_CASE("Gate node","[Unit Test: Lock open]"){

    uint8_t mac[]={1,2,3,4,5,6};
    gateSetUp();
    lock_system_message_t message={0};
    message.msg_type=MSG_TYPE_COMMAD;
    message.cmd=LOCK_SYSTEM_COMMAND_OPEN_LOCK;
    
    msg.msgReceivedCallback(mac,(const uint8_t*)&message,sizeof(lock_system_message_t));

}