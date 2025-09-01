// File: components/esp_now_transport/include/esp_now_transport.h

#ifndef ESP_NOW_TRANSPORT_H
#define ESP_NOW_TRANSPORT_H

#include "esp_err.h"
#include "esp_now.h"
#include <stdint.h>
#include <stdbool.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

// Maximum data payload size (ESP-NOW max is 250, minus our header)
#define ESP_NOW_TRANSPORT_MAX_DATA_LEN  (ESP_NOW_MAX_DATA_LEN - 16)

/**
 * @brief Callback function type for device discovery
 * 
 * @param mac_addr MAC address of the discovered device
 */
typedef void (*esp_now_transport_device_discovered_cb_t)(const uint8_t *mac_addr);

/**
 * @brief Callback function type for discovery acknowledgment
 * 
 * @param mac_addr MAC address of the device that discovered us
 */
typedef void (*esp_now_transport_discovery_ack_cb_t)(const uint8_t *mac_addr);

/**
 * @brief Callback function type for data reception
 * 
 * @param mac_addr Source MAC address
 * @param data Received data
 * @param len Data length
 */
typedef void (*esp_now_transport_data_received_cb_t)(const uint8_t *mac_addr, const uint8_t *data, size_t len);

/**
 * @brief Callback function type for send completion
 * 
 * @param mac_addr Target MAC address
 * @param success True if send was successful
 */
typedef void (*esp_now_transport_send_done_cb_t)(const uint8_t *mac_addr, bool success);

/**
 * @brief ESP-NOW transport callback structure
 */
typedef struct {
    esp_now_transport_device_discovered_cb_t on_device_discovered;    ///< Called when a new device is discovered
    esp_now_transport_discovery_ack_cb_t on_discovery_ack;           ///< Called when we receive discovery acknowledgment
    esp_now_transport_data_received_cb_t on_data_received;           ///< Called when data is received
    esp_now_transport_send_done_cb_t on_send_done;                   ///< Optional: Called when send operation completes
} esp_now_transport_callbacks_t;

/**
 * @brief ESP-NOW transport configuration
 */

 typedef struct {
//    esp_now_transport_callbacks_t callbacks;        ///< Callback functions
    uint8_t wifi_channel;                           ///< WiFi channel to use (1-14)
//  uint32_t discovery_interval_ms;                 ///< Discovery broadcast interval in milliseconds
//    uint8_t total_discovery_attempts;
} esp_now_transport_config_t;


typedef struct {
    /*
    esp_err_t (*set_esp_now_device_discovery_cb)(esp_now_transport_device_discovered_cb_t);        ///< Callback functions
    esp_err_t (*set_esp_now_device_discovery_ack_cb)(esp_now_transport_discovery_ack_cb_t);        ///< Callback functions
    esp_err_t (*set_esp_now_device_data_rcv_cb)(esp_now_transport_data_received_cb_t);        ///< Callback functions
    esp_err_t (*set_esp_now_device_data_sent_cb)(esp_now_transport_send_done_cb_t);        ///< Callback functions
    
    */
    esp_now_transport_callbacks_t callbacks;
    
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


    /**
     * @brief Send data to a specific device
     * 
     * @param mac_addr Target device MAC address
     * @param data Data to send
     * @param len Data length (must be <= (*esp_now_TRANSPORT_MAX_DATA_LEN)
     * @return ESP_OK on success
     */
    esp_err_t (*esp_now_transport_send_data)(const uint8_t *mac_addr, const uint8_t *data, size_t len);

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



}esp_now_trasnsport_interface_t;







/**
 * @brief Initialize ESP-NOW transport
 * 
 * @param config Configuration structure
 * @return ESP_OK on success
 */
esp_now_trasnsport_interface_t* esp_now_transport_init(const esp_now_transport_config_t *config);

/**
     * @brief Deinitialize ESP-NOW transport
     * 
     * @return ESP_OK on success
     */
esp_err_t esp_now_transport_deinit(void);





//#ifdef __cplusplus
//}
//#endif

#endif // ESP_NOW_TRANSPORT_H