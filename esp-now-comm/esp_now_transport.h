// File: components/esp_now_transport/include/esp_now_transport.h

#ifndef ESP_NOW_TRANSPORT_H
#define ESP_NOW_TRANSPORT_H

#include "esp_err.h"

#include <stdint.h>
#include <stdbool.h>
#include "esp_now_common_interface.h"
#include "esp_now_discovery_interface.h"
#include "esp_now_msg_interface.h"
//#ifdef __cplusplus
//extern "C" {
//#endif





   

// Maximum data payload size (ESP-NOW max is 250, minus our header)
#define ESP_NOW_TRANSPORT_MAX_DATA_LEN  (250 - 16)

 typedef struct {
//    esp_now_transport_callbacks_t callbacks;        ///< Callback functions
    uint8_t wifi_channel;                           ///< WiFi channel to use (1-14)
//  uint32_t discovery_interval_ms;                 ///< Discovery broadcast interval in milliseconds
//    uint8_t total_discovery_attempts;
} esp_now_transport_config_t;



//Package for the discovery user
typedef struct {

    esp_now_transport_discovery_interface_t discovery;
    esp_now_transport_common_interface_t common;

}esp_now_trasnsport_discovery_package_t;



//Package for the msg user
typedef struct {

    esp_now_transport_msg_interface_t msg;
    esp_now_transport_common_interface_t common;

}esp_now_trasnsport_msg_package_t;





/**
 * @brief Initialize ESP-NOW transport
 * 
 * @param config Configuration structure
 * @return ESP_OK on success
 */
esp_err_t esp_now_transport_init(const esp_now_transport_config_t *config);

/**
     * @brief Deinitialize ESP-NOW transport
     * 
     * @return ESP_OK on success
     */
esp_err_t esp_now_transport_deinit(void);


esp_now_trasnsport_discovery_package_t* esp_now_transport_get_discovery_interface();
esp_now_trasnsport_msg_package_t* esp_now_transport_get_msg_interface();


//#ifdef __cplusplus
//}
//#endif

#endif // ESP_NOW_TRANSPORT_H