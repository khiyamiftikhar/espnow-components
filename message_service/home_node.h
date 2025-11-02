#ifndef HOME_NODE_H
#define HOME_NODE_H

#include "esp_err.h"
#include "home_node_interfaces.h"
#include "common_interface.h"
#include "event_system_adapter.h"

DECLARE_EVENT_ADAPTER(HOME_NODE);


#define HOME_NODE_ROUTINE_
//These are the interfaces that it requires

typedef struct{
    user_output_interface_t* user_output;
    node_msg_interface_t* msg_interface;
    node_white_list_interface_t* white_list;
    gate_node_id_interface_t* gate_node_id;
}home_node_config_t;


//It is the handlers which it provides to be assigned to the callbacks





esp_err_t home_node_send_command(user_command_t cmd);

esp_err_t home_node_service_create(home_node_config_t* config);



#endif