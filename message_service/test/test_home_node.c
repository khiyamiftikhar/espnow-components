#include <stdio.h>
#include <string.h>
#include "home_node.h"
#include "unity.h"
#include "esp_log.h"
#include "esp_err.h"
#include <inttypes.h>
#include "message_def.h"
static const char* TAG="test Home node";



static home_node_config_t config;
static user_interaction_interface_t user_interaction;
static node_msg_interface_t msg_interface;
static node_white_list_interface_t white_list;
static gate_node_id_interface_t gate_node_id;

static esp_err_t inform_command_status(bool success){

    ESP_LOGI(TAG,"command sent %d",success);
    return 0;
}

static esp_err_t inform_lock_status(lock_status_t status){

    ESP_LOGI(TAG,"lock status %d",status);
    return 0;
}

static esp_err_t send_message(const uint8_t *mac_addr, const uint8_t *data, size_t len){

    ESP_LOGI(TAG,"sending message");
    return 0;
}

static bool is_in_whitelist(const uint8_t *mac_addr){

    return true;
}

static esp_err_t get_gate_node_mac(uint8_t* mac){

    ESP_LOGI(TAG,"giving gate node mac");
    uint8_t gate_mac[]={1,2,3,4,5,6};
    memcpy(mac,gate_mac,sizeof(gate_mac));
    return 0;

}

static void homeSetUp(){
    
    user_interaction.inform_command_status=inform_command_status;
    user_interaction.inform_lock_status=inform_lock_status;
    msg_interface.send_msg=send_message;
    white_list.is_in_whitelist=is_in_whitelist;
    gate_node_id.get_gate_node_mac=get_gate_node_mac;

    config.gate_node_id=&gate_node_id;
    config.msg_interface=&msg_interface;
    config.user_interaction=&user_interaction;
    config.white_list=&white_list;
    home_node_servive_create(&config);


}


TEST_CASE("Home node","[Unit Test: LOCK close]"){

    uint8_t mac[]={1,2,3,4,5,6};
    homeSetUp();
    /*
    lock_system_message_t message={0};
    message.msg_type=MSG_TYPE_COMMAD;
    message.cmd=LOCK_SYSTEM_COMMAND_CLOSE_LOCK;
    */
    user_interaction.user_command_callback(USER_COMMAND_LOCK_CLOSE);
    //msg.msgReceivedCallback(mac,(const uint8_t*)&message,sizeof(lock_system_message_t));

}

TEST_CASE("Home node","[Unit Test: Lock open]"){

    uint8_t mac[]={1,2,3,4,5,6};
    homeSetUp();
    /*
    lock_system_message_t message={0};
    message.msg_type=MSG_TYPE_COMMAD;
    message.cmd=LOCK_SYSTEM_COMMAND_OPEN_LOCK;
    */
    user_interaction.user_command_callback(USER_COMMAND_LOCK_OPEN);

}