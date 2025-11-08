#ifndef PEER_MANAGER_INTERFACE_H
#define PEER_MANAGER_INTERFACE_H


#include "stdint.h"
#include "esp_err.h"
   
/*
 * @brief ESP-NOW transport configuration
 */

typedef struct {
    /**
     * @brief Add a peer device manually
     * 
     * @param mac_addr MAC address of the peer to add
     * @return ESP_OK on success
     */
    esp_err_t (*esp_now_transport_add_peer)(const uint8_t *mac_addr);

    /**
     * @brief Remove a peer device
     * 
     * @param mac_addr MAC address of the peer to remove
     * @return ESP_OK on success
     */
    esp_err_t (*esp_now_transport_remove_peer)(const uint8_t *mac_addr);

    /**
     * @brief Check if a peer exists
     * 
     * @param mac_addr MAC address to check
     * @return true if peer exists, false otherwise
     */
    bool (*esp_now_transport_is_peer_exist)(const uint8_t *mac_addr);



}esp_now_peer_manager_interface_t;










#endif