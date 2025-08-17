#ifndef COMMON_DEF_H
#define COMMON_DEF_H


#include "stdbool.h"
#include "common_interface.h"

typedef enum{
    MSG_TYPE_COMMAD,
    MSG_TYPE_STATUS
}lock_system_message_type_t;

typedef enum{
    LOCK_SYSTEM_COMMAND_OPEN_LOCK,
    LOCK_SYSTEM_COMMAND_CLOSE_LOCK,
    LOCK_SYSTEM_COMMAND_LOCK_STATUS
}lock_system_command_type_t;


typedef struct {
    lock_system_message_type_t msg_type;
    lock_system_command_type_t cmd;
    lock_system_lock_status_t lock_status;
    

}lock_system_message_t;









#endif