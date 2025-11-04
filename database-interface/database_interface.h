#ifndef DATABASE_INTERFACE_H
#define DATABASE_INTERFACE_H


#include "stdint.h"

typedef struct{

        //Check if any deivce is added to the white list
        bool (*is_white_listed)(const uint8_t *mac_addr);

}database_interface_t;



#endif