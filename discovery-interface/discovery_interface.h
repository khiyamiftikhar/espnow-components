#ifndef DISCOVERY_INTERFACE_H
#define DISCOVERY_INTERFACE_H


#include "stdint.h"
#include "esp_err.h"



typedef void (*esp_now_transport_device_discovered_cb_t)(const uint8_t *mac_addr);

/**
 * @brief Callback function type for discovery acknowledgment
 * 
 * @param mac_addr MAC address of the device that discovered us
 */
typedef void (*esp_now_transport_discovery_ack_cb_t)(const uint8_t *mac_addr);

/**
 * @brief ESP-NOW transport callback structure
 */
typedef struct {
    esp_now_transport_device_discovered_cb_t on_device_discovered;    ///< Called when a new device is discovered
    esp_now_transport_discovery_ack_cb_t on_discovery_ack;           ///< Called when we receive discovery acknowledgment
} esp_now_transport_discovery_callbacks_t;

/**
 * @brief ESP-NOW transport configuration
 */

typedef struct {
    
    esp_err_t (*set_esp_now_device_discovery_cb)(esp_now_transport_device_discovered_cb_t);        ///< Callback functions
    esp_err_t (*set_esp_now_device_discovery_ack_cb)(esp_now_transport_discovery_ack_cb_t);        ///< Callback functions
    /**
     * @brief Start device discovery broadcasting
     * 
     * Starts periodic broadcasting of discovery messages to find other devices.
     * When other devices receive these broadcasts, they will send acknowledgments.
     * 
     * @return ESP_OK on success
     */
    esp_err_t (*esp_now_transport_send_discovery)(void);

    /**
     * @brief Stop device discovery broadcasting
     * 
     * @return ESP_OK on success
     */
   // esp_err_t (*esp_now_transport_stop_discovery)(void);
   /**
     * @brief Acknowlege the discovery, so that the broadcater may stpo uf desires
     * 
     * 
     * Adds the source address in the internal message. The data payload is not used
     * 
     * @return ESP_OK on success
     */
    esp_err_t (*esp_now_transport_send_discovery_ack)(const uint8_t* mac);


}esp_now_transport_discovery_interface_t;






#endif