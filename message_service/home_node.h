#ifndef HOME_NODE_H
#define HOME_NODE_H

#include "esp_err.h"
#include "home_node_interfaces.h"
#include "common_interface.h"



typedef struct{
    user_interaction_interface_t* user_interaction;
    node_msg_interface_t* msg_interface;
    node_white_list_interface_t* white_list;
    gate_node_id_interface_t* gate_node_id;
}home_node_config_t;


esp_err_t home_node_servive_create(home_node_config_t* config);



#endif