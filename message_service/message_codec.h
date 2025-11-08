#ifndef MESSAGE_CODEC_H
#define MESSAGE_CODEC_H


#include "message_def.h"
#include "esp_now_msg_interface.h"
#include "event_system_adapter.h"

DECLARE_EVENT_ADAPTER(MESSAGE_CODEC);


#define MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_OPEN_GATE             0
#define MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_CLOSE_GATE            1
#define MESSAGE_SERVICE_ROUTINE_EVENT_COMMAND_SEND_GATE_STATUS      2
#define MESSAGE_SERVICE_ROUTINE_EVENT_GATE_STATUS_ARRIVED           3

//These are the interfaces that it requires




typedef enum{
    LOCK_SYSTEM_COMMAND_OPEN_LOCK,
    LOCK_SYSTEM_COMMAND_CLOSE_LOCK,
    LOCK_SYSTEM_COMMAND_LOCK_STATUS
}lock_system_command_type_t;

typedef enum{
    LOCK_STATUS_OPEN,
    LOCK_STATUS_CLOSED,
    LOCK_STATUS_OPENING,
    LOCK_STATUS_CLOSING,
    LOCK_STATUS_CLOSED_IDLE,    //These new states added for automotive lock actuator
    LOCK_STATUS_OPENDED_IDLE,

}lock_system_lock_status_t;


typedef struct{
    esp_now_transport_msg_interface_t* msg_interface;
    database_interface_t* database_interface;
    
}message_codec_config_t;


//It is the handlers which it provides to be assigned to the callbacks





esp_err_t message_codec_send_command(uint8_t* mac_addr,lock_system_command_type_t command);


esp_err_t message_codec_send_status(uint8_t* mac_addr,lock_system_lock_status_t status);
esp_err_t lock_system_message_codec_init(message_codec_config_t* config);



#endif