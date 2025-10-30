// File: components/esp_now_transport/esp_now_transport.c

#include "esp_now_transport.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_crc.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "string.h"

static const char *TAG = "ESP_NOW_TRANSPORT";

// Broadcast MAC address for discovery
static const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

DEFINE_EVENT_ADAPTER(ESPNOW_TRANSPORT);

// Message types
typedef enum {
    MSG_TYPE_DISCOVERY_BROADCAST = 0x01,
    MSG_TYPE_DISCOVERY_ACK = 0x02,
    MSG_TYPE_DATA = 0x03
} esp_now_msg_type_t;

// Internal message structure
typedef struct {
    uint8_t type;
    uint8_t src_mac[6];
    uint16_t payload_len;
    uint32_t crc;
    uint8_t payload[];
} __attribute__((packed)) esp_now_internal_msg_t;

// Component state
static struct {
    bool initialized;
    //esp_now_transport_callbacks_t callbacks;
    //TimerHandle_t discovery_timer;
    //bool discovery_active;
    //uint32_t discovery_interval_ms;
    //uint8_t total_discovery_attempts;        //How many times tried to send broadcast message
//    uint8_t discovery_attempt_count;
    esp_now_trasnsport_interface_t interface;
    //esp_now_transport_callbacks_t callbacks;
} esp_now_state = {0};

// Forward declarations
static void esp_now_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);
static void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status);
//static void discovery_timer_callback(TimerHandle_t timer);



esp_err_t esp_now_transport_deinit(void)
{
    if (!esp_now_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    //esp_now_transport_stop_discovery();
    
    /*
    if (esp_now_state.discovery_timer) {
        xTimerDelete(esp_now_state.discovery_timer, portMAX_DELAY);
        esp_now_state.discovery_timer = NULL;
    }
    */
    esp_now_deinit();
    esp_now_state.initialized = false;

    ESPNOW_TRANSPORT_unregister_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_DISCOVERY_INCOMING);
    ESPNOW_TRANSPORT_unregister_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_DISCOVERY_ACK_INCOMING);
    ESPNOW_TRANSPORT_unregister_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_MSG_RECEIVED);
    ESPNOW_TRANSPORT_unregister_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_DISCOVERY_ACK_INCOMING);
    
    ESP_LOGI(TAG, "ESP-NOW transport deinitialized");
    return ESP_OK;
}

static esp_err_t esp_now_transport_send_discovery(void)
{
    if (!esp_now_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    /*
    if (esp_now_state.discovery_active) {
        ESP_LOGW(TAG, "Discovery already active");
        return ESP_OK;
    }*/

    esp_now_internal_msg_t msg;
    msg.type = MSG_TYPE_DISCOVERY_BROADCAST;
    esp_wifi_get_mac(ESP_IF_WIFI_STA, msg.src_mac);
    msg.payload_len = 0;
    msg.crc = 0;

    esp_err_t ret = esp_now_send(BROADCAST_MAC, (uint8_t*)&msg, sizeof(msg));
    //ESP_LOGI(TAG,"D ret %d",ret);
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "Discovery broadcast sent");
    } else {
        ESP_LOGW(TAG, "Failed to send discovery broadcast: %s", esp_err_to_name(ret));
    }
    //esp_now_state.discovery_active = true;
    


    /*
    if (xTimerStart(esp_now_state.discovery_timer, 0) != pdPASS) {
        ESP_LOGE(TAG, "Failed to start discovery timer");
        esp_now_state.discovery_active = false;
        return ESP_FAIL;
    }*/

    //ESP_LOGI(TAG, "Discovery sent");
    return ESP_OK;
}

/*

static esp_err_t esp_now_transport_stop_discovery(void)
{
    if (!esp_now_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_now_state.discovery_active = false;
    //esp_now_transport_stop_discovery();
    
    
    if (esp_now_state.discovery_timer) {
        xTimerStop(esp_now_state.discovery_timer, 0);
    }

    ESP_LOGI(TAG, "Discovery stopped");
    return ESP_OK;
}*/

static esp_err_t esp_now_transport_send_data(const uint8_t *mac_addr, const uint8_t *data, size_t len)
{
    if (!esp_now_state.initialized || !mac_addr || !data || len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    if (len > ESP_NOW_TRANSPORT_MAX_DATA_LEN) {
        ESP_LOGE(TAG, "Data too large: %zu > %d", len, ESP_NOW_TRANSPORT_MAX_DATA_LEN);
        return ESP_ERR_INVALID_SIZE;
    }

    
    // Check if peer exists - application must add peers explicitly
    if (!esp_now_is_peer_exist(mac_addr)) {
        ESP_LOGE(TAG, "Peer " MACSTR " not found. Add peer first.", MAC2STR(mac_addr));
        
        return ESP_ERR_ESPNOW_NOT_FOUND;
    }

    // Use static allocation for message
    uint8_t msg_buffer[sizeof(esp_now_internal_msg_t) + ESP_NOW_TRANSPORT_MAX_DATA_LEN];
    esp_now_internal_msg_t *msg = (esp_now_internal_msg_t*)msg_buffer;

    msg->type = MSG_TYPE_DATA;
    esp_wifi_get_mac(ESP_IF_WIFI_STA, msg->src_mac);
    msg->payload_len = len;
    memcpy(msg->payload, data, len);
    msg->crc = esp_crc32_le(0, msg->payload, len);

    size_t msg_size = sizeof(esp_now_internal_msg_t) + len;
    //IT is assumed that the library copies to internal buffer
    esp_err_t ret = esp_now_send(mac_addr, msg_buffer, msg_size);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send data: %s", esp_err_to_name(ret));
    }

    return ret;
}

static esp_err_t esp_now_transport_add_peer(const uint8_t *mac_addr)
{
    if (!esp_now_state.initialized || !mac_addr) {
        return ESP_ERR_INVALID_ARG;
    }

    if (esp_now_is_peer_exist(mac_addr)) {
        return ESP_OK;  // Already exists
    }

    esp_now_peer_info_t peer_info = {0};
    memcpy(peer_info.peer_addr, mac_addr, 6);
    peer_info.channel = 0;  // Use current channel
    peer_info.ifidx = ESP_IF_WIFI_STA;
    peer_info.encrypt = false;

    return esp_now_add_peer(&peer_info);
}

static esp_err_t esp_now_transport_remove_peer(const uint8_t *mac_addr)
{
    if (!esp_now_state.initialized || !mac_addr) {
        return ESP_ERR_INVALID_ARG;
    }

    return esp_now_del_peer(mac_addr);
}

bool esp_now_transport_is_peer_exist(const uint8_t *mac_addr)
{
    if (!esp_now_state.initialized || !mac_addr) {
        return false;
    }

    return esp_now_is_peer_exist(mac_addr);
}

static esp_err_t esp_now_transport_send_discovery_ack(const uint8_t* mac){

// Send acknowledgment (add peer temporarily just for this response)
        esp_now_internal_msg_t ack_msg = {0};
        ack_msg.type = MSG_TYPE_DISCOVERY_ACK;
        esp_wifi_get_mac(ESP_IF_WIFI_STA, ack_msg.src_mac);
        ack_msg.payload_len = 0;
        ack_msg.crc = 0;

        // Temporarily add peer to send ACK, then let application decide
        bool peer_existed = esp_now_is_peer_exist(mac);
        if (!peer_existed) {
            esp_now_transport_add_peer(mac);
        }
        esp_now_send(mac, (uint8_t*)&ack_msg, sizeof(ack_msg));
        if (!peer_existed) {
            esp_now_transport_remove_peer(mac);
        }

        return 0;
}


// Internal callback functions
static void esp_now_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{

    ESP_LOGI(TAG,"received");
    if (!recv_info || !data || len < sizeof(esp_now_internal_msg_t)) {
        return;
    }

    ESP_LOGI(TAG,"received not returned early");
    esp_now_internal_msg_t *msg = (esp_now_internal_msg_t*)data;
    
    // Validate message
    if (len != sizeof(esp_now_internal_msg_t) + msg->payload_len) {
        ESP_LOGW(TAG, "Invalid message length");
        return;
    }
    

    // Only validate CRC for data messages (discovery messages have CRC = 0)
    if (msg->type == MSG_TYPE_DATA && msg->payload_len > 0) {
        uint32_t calc_crc = esp_crc32_le(0, msg->payload, msg->payload_len);
        if (msg->crc != calc_crc) {
            ESP_LOGW(TAG, "CRC mismatch");
            return;
        }
    }

    switch (msg->type) {
        case MSG_TYPE_DISCOVERY_BROADCAST:
            ESP_LOGI(TAG, "Received discovery broadcast from " MACSTR, MAC2STR(msg->src_mac));
            
            // Notify application - let it decide whether to add peer
            ESPNOW_TRANSPORT_post_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_DISCOVERY_INCOMING,msg->src_mac,sizeof(msg->src_mac));
            break;
            

        case MSG_TYPE_DISCOVERY_ACK:
            ESP_LOGI(TAG, "Received discovery ACK from " MACSTR, MAC2STR(msg->src_mac));
            ESPNOW_TRANSPORT_post_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_DISCOVERY_ACK_INCOMING,msg->src_mac,sizeof(msg->src_mac));
            // Don't add peer automatically - let application decide
            // Notify application that we were discovered by this device
            break;

        case MSG_TYPE_DATA:
            ESP_LOGD(TAG, "Received data from " MACSTR, MAC2STR(msg->src_mac));
            
            // Notify application
            uint8_t payload_length=msg->payload_len;
            //Variable Length Array
            uint8_t msg_buffer[sizeof(espnow_msg_recv_t) + ESP_NOW_TRANSPORT_MAX_DATA_LEN];

            espnow_msg_recv_t* message=(espnow_msg_recv_t*)msg_buffer;

            memcpy(message->payload,msg->payload,msg->payload_len);
            memcpy(message->src_mac,msg->src_mac,sizeof(message->src_mac));
            message->payload_len=msg->payload_len;

            
            ESPNOW_TRANSPORT_post_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_MSG_RECEIVED,message,sizeof(msg_buffer));
            esp_now_state.interface.callbacks.on_data_received(msg->src_mac, msg->payload, msg->payload_len);
            
            break;

        default:
            ESP_LOGW(TAG, "Unknown message type: %d", msg->type);
            break;
    }
}

static void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    bool success=false;
    if (status == ESP_NOW_SEND_SUCCESS) {
        success=true;
        ESP_LOGI(TAG, "Send success to " MACSTR "", MAC2STR(mac_addr));
    } else {
        ESP_LOGI(TAG, "Send failed to " MACSTR"", MAC2STR(mac_addr));
    }

    // Notify application if callback is set
    
    espnow_msg_sent_status_t msg;
    msg.success=success;
    memcpy(msg.dest_mac,mac_addr,sizeof(msg.dest_mac));
    ESPNOW_TRANSPORT_post_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_MSG_SENT,&msg,sizeof(espnow_msg_sent_status_t));

    
}

/*
static void discovery_timer_callback(TimerHandle_t timer)
{
   
    if (!esp_now_state.discovery_active) {
        return;
    }

    
    //Check if attempts made are more than told
    esp_now_state.discovery_attempt_count++;   
    //If yes then stop discovery
    if(esp_now_state.discovery_attempt_count>esp_now_state.total_discovery_attempts){
        esp_now_transport_stop_discovery();
    }
    // Use static allocation for discovery message
    static esp_now_internal_msg_t msg;
    msg.type = MSG_TYPE_DISCOVERY_BROADCAST;
    esp_wifi_get_mac(ESP_IF_WIFI_STA, msg.src_mac);
    msg.payload_len = 0;
    msg.crc = 0;

    esp_err_t ret = esp_now_send(BROADCAST_MAC, (uint8_t*)&msg, sizeof(msg));
    if (ret == ESP_OK) {
        ESP_LOGD(TAG, "Discovery broadcast sent");
    } else {
        ESP_LOGW(TAG, "Failed to send discovery broadcast: %s", esp_err_to_name(ret));
    }
}*/

/*
esp_err_t set_esp_now_device_discovery_cb(esp_now_transport_device_discovered_cb_t cb){

    if(cb!=NULL)
        esp_now_state.callbacks.on_device_discovered=cb;
    return 0;
}
esp_err_t set_esp_now_device_discovery_ack_cb(esp_now_transport_discovery_ack_cb_t cb){
    if(cb!=NULL)
        esp_now_state.callbacks.on_discovery_ack=cb;
    return 0;
}
esp_err_t set_esp_now_device_data_rcv_cb(esp_now_transport_data_received_cb_t cb){
    if(cb!=NULL)
        esp_now_state.callbacks.on_data_received=cb;
    return 0;
}
esp_err_t set_esp_now_device_data_sent_cb(esp_now_transport_send_done_cb_t cb){
    if(cb!=NULL)
        esp_now_state.callbacks.on_send_done=cb;
    return 0;
}

*/




esp_now_trasnsport_interface_t* esp_now_transport_init(const esp_now_transport_config_t *config)
{
    if (esp_now_state.initialized) {
        ESP_LOGW(TAG, "Already initialized");
        return NULL;
    }

    /*
    if (!config || !config->callbacks.on_device_discovered || 
        !config->callbacks.on_discovery_ack || !config->callbacks.on_data_received) {
        ESP_LOGE(TAG, "Invalid config or missing callbacks");
        return NULL;
    }

    */
    // Initialize ESP-NOW
    esp_err_t ret = esp_now_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ESP-NOW: %s", esp_err_to_name(ret));
        return NULL;
    }

    // Register callbacks
    ret = esp_now_register_recv_cb(esp_now_recv_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register recv callback: %s", esp_err_to_name(ret));
        esp_now_deinit();
        return NULL;
    }

    ret = esp_now_register_send_cb(esp_now_send_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register send callback: %s", esp_err_to_name(ret));
        esp_now_deinit();
        return NULL;
    }

    // Add broadcast peer for discovery
    esp_now_peer_info_t peer_info = {0};
    memcpy(peer_info.peer_addr, BROADCAST_MAC, 6);
    peer_info.channel = config->wifi_channel;
    peer_info.ifidx = ESP_IF_WIFI_STA;
    peer_info.encrypt = false;

    ret = esp_now_add_peer(&peer_info);
    if (ret != ESP_OK && ret != ESP_ERR_ESPNOW_EXIST) {
        ESP_LOGE(TAG, "Failed to add broadcast peer: %s", esp_err_to_name(ret));
        esp_now_deinit();
        return NULL;
    }

    // Store configuration
    
    //esp_now_state.discovery_interval_ms = config->discovery_interval_ms;
    //esp_now_state.interface.callbacks = config->callbacks;
    esp_now_state.interface.esp_now_transport_add_peer=esp_now_transport_add_peer;
    //esp_now_state.interface.esp_now_transport_deinit=esp_now_transport_deinit;
    esp_now_state.interface.esp_now_transport_is_peer_exist=esp_now_transport_is_peer_exist;
    esp_now_state.interface.esp_now_transport_remove_peer=esp_now_transport_remove_peer;
    esp_now_state.interface.esp_now_transport_send_data=esp_now_transport_send_data;
    esp_now_state.interface.esp_now_transport_send_discovery=esp_now_transport_send_discovery;
    esp_now_state.interface.esp_now_transport_send_discovery_ack=esp_now_transport_send_discovery_ack;
    //esp_now_state.interface.esp_now_transport_stop_discovery=esp_now_transport_stop_discovery;


    //Earlier this was accomplised using callbacks
    //Now this source posts events

    ESPNOW_TRANSPORT_register_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_DISCOVERY_INCOMING,NULL);
    ESPNOW_TRANSPORT_register_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_DISCOVERY_ACK_INCOMING,NULL);
    ESPNOW_TRANSPORT_register_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_MSG_RECEIVED,NULL);
    ESPNOW_TRANSPORT_register_event(ESPNOW_TRANSPORT_ROUTINE_EVENT_DISCOVERY_ACK_INCOMING,NULL);
    // Create discovery timer
    /*
    esp_now_state.discovery_timer = xTimerCreate(
        "discovery_timer",
        pdMS_TO_TICKS(esp_now_state.discovery_interval_ms),
        pdTRUE,  // Auto-reload
        NULL,
        discovery_timer_callback
    );

    if (!esp_now_state.discovery_timer) {
        ESP_LOGE(TAG, "Failed to create discovery timer");
        esp_now_deinit();
        return NULL;
    }*/

    esp_now_state.initialized = true;
    ESP_LOGI(TAG, "ESP-NOW transport initialized successfully");
    return &esp_now_state.interface;
}
