#ifndef MESSAGE_INTERFACE_H
#define MESSAGE_INTERFACE_H


#include "stdint.h"
#include "esp_err.h"
#include "stdbool.h"



typedef struct{
    uint8_t src_mac[6];
    uint16_t payload_len;
    uint8_t payload[];      //Note not a pointer, and last member bcz mem after the struct size will be used by the payload
}espnow_msg_recv_t;

typedef struct{
    uint8_t dest_mac[6];
    bool success;
}espnow_msg_sent_status_t;




   

// Maximum data payload size (ESP-NOW max is 250, minus our header)
#define ESP_NOW_TRANSPORT_MAX_DATA_LEN  (ESP_NOW_MAX_DATA_LEN - 16)


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
    esp_now_transport_data_received_cb_t on_data_received;           ///< Called when data is received
    esp_now_transport_send_done_cb_t on_send_done;                   ///< Optional: Called when send operation completes
} esp_now_transport_msg_callbacks_t;

/**
 * @brief ESP-NOW transport configuration
 */

 
typedef struct {
    esp_err_t (*set_esp_now_device_data_rcv_cb)(esp_now_transport_data_received_cb_t);        ///< Callback functions
    esp_err_t (*set_esp_now_device_data_sent_cb)(esp_now_transport_send_done_cb_t);        ///< Callback functions
        
    /**
     * @brief Send data to a specific device
     * 
     * @param mac_addr Target device MAC address
     * @param data Data to send
     * @param len Data length (must be <= (*esp_now_TRANSPORT_MAX_DATA_LEN)
     * @return ESP_OK on success
     */
    esp_err_t (*esp_now_transport_send_data)(const uint8_t *mac_addr, const uint8_t *data, size_t len);

    

}esp_now_transport_msg_interface_t;

#endif