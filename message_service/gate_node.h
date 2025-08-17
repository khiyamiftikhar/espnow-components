#ifndef GATE_NODE_H
#define GATE_NODE_H
/*This is application level source. it is an extension of main, just to separate the divide and
separate different 'main' responsibilities. So it does not implement an interface. because an 
interface is provided by some higher level compoenent and then implemented by lower level which is
then injected to the higher level component. It is the highest level component. it defines its own
responsiilites and not anyone else, and then it implements what it defines as its rrole
However it defines interfaces for other components that must be provided/injcted to it.

This source has following purposes

-Interpret a command and take appropriate action as disctated in command
    which could be
     -Open Lock
     -Close Lock
     -Send Lock status

*/


#include "esp_err.h"
#include "stdbool.h"
#include "common_interface.h"

//#include "esp_now_transport.h"











//This is the interface it needs to open/close or check status of the lock

typedef struct{
    esp_err_t (*set_lock_open)();
    esp_err_t (*set_lock_close)();
    lock_system_lock_status_t (*get_lock_status)();
}gate_node_lock_interface_t;



typedef struct {
    node_msg_interface_t* msg;
    node_white_list_interface_t* list;
    gate_node_lock_interface_t* lock;

}gate_node_config_t;


esp_err_t gate_node_init(gate_node_config_t* config);


#endif