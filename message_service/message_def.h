#ifndef MESSAGE_DEF_H
#define MESSAGE_DEF_H


#include "stdbool.h"



typedef enum{
    LOCK_STATUS_OPEN,
    LOCK_STATUS_CLOSED,
    LOCK_STATUS_OPENING,
    LOCK_STATUS_CLOSING,
    LOCK_STATUS_CLOSED_IDLE,    //These new states added for automotive lock actuator
    LOCK_STATUS_OPENDED_IDLE,

}lock_system_lock_status_t;



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
    uint32_t tid;           //Uniqely identify the transcation id

}lock_system_message_t;









#endif