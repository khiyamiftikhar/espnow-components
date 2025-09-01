#ifndef HOME_NODE_INTERFACE_H
#define HOME_NODE_INTERFACE_H

#include "stdint.h"
#include "stdbool.h"
#include "esp_err.h"




typedef enum{
    USER_COMMAND_LOCK_OPEN,
    USER_COMMAND_LOCK_CLOSE,
    USER_COMMAND_LOCK_STATUS
}user_command_t;




typedef enum{
    STATUS_OPEN,
    STATUS_CLOSED,
}lock_status_t;




typedef esp_err_t (*user_command_callback)(user_command_t);

//Wheneven user sends a command the handler inside this must be invoked
typedef struct{

    //When an object of this interface implementation will be created and injected in to the
    //top level source (gate_node and home_node), they will assign their callback handlers to this member
    
    //When reply is  received from the gate
    esp_err_t (*inform_lock_status)(lock_status_t status);
    //When command is succesfully sent
    esp_err_t (*inform_command_status)(bool success);  

}user_output_interface_t;







//interface for espnow communucation


typedef struct{
    esp_err_t (*get_gate_node_mac)(uint8_t* gate_node_mac);

}gate_node_id_interface_t;

#endif