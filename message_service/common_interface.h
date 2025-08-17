#ifndef COMMON_INTERFACE_H
#define COMMON_INTERFACE_H
#include "stdbool.h"
#include "stdint.h"
#include "esp_err.h"

//This is the interface it needs to check the record


typedef enum{
    LOCK_STATUS_OPEN,
    LOCK_STATUS_CLOSED,
    LOCK_STATUS_OPENING,
    LOCK_STATUS_CLOSING,

}lock_system_lock_status_t;



typedef struct{
    //When an object of this interface implementation will be created and injected in to the
    //top level source (gate_node and home_node), they will assign their callback handlers to this member
    //Callback invokked when msg received through espnow
    void (*msgReceivedCallback)(const uint8_t *mac_addr, const uint8_t *data, size_t len);
    //Callback invoked when message sent successfullt
    //destination mac address
    void (*msgSentCallback)(const uint8_t *mac_addr, bool success);
    //send message through espnow
    esp_err_t  (*send_msg)(const uint8_t *mac_addr, const uint8_t *data, size_t len);
   
}node_msg_interface_t;

typedef struct{
    
    bool (*is_in_whitelist)(const uint8_t *mac_addr);
}node_white_list_interface_t;


#endif