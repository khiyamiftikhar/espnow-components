// File: components/peer_registry/peer_registry.c

#include "peer_registry.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "string.h"

static const char *TAG = "PEER_REGISTRY";

// Static registry storage - using device_id as direct index for O(1) lookup
static struct {
    bool initialized;
    uint16_t max_peers;
    uint16_t peer_count;
    peer_registry_entry_t peers[PEER_REGISTRY_MAX_PEERS];
    // Hash table for fast ID lookup: index = (device_id - 1) % PEER_REGISTRY_MAX_PEERS
    uint16_t id_to_slot[PEER_REGISTRY_MAX_PEERS];  // Maps device_id to slot index
    peer_registry_interface_t interface;
} registry_state = {0};

// Helper function to get slot index from device ID using simple hash
static inline uint16_t device_id_to_slot_hash(peer_device_id_t device_id)
{
    if (device_id == PEER_REGISTRY_INVALID_ID) {
        return PEER_REGISTRY_MAX_PEERS;  // Invalid slot
    }
    return (device_id - 1) % PEER_REGISTRY_MAX_PEERS;
}

// Helper function to find peer slot by device ID - O(1) average case
static int find_peer_slot_by_id(peer_device_id_t device_id)
{
    if (device_id == PEER_REGISTRY_INVALID_ID) {
        return -1;
    }

    // Try hash slot first
    uint16_t hash_slot = device_id_to_slot_hash(device_id);
    uint16_t slot = registry_state.id_to_slot[hash_slot];
    
    if (slot < PEER_REGISTRY_MAX_PEERS && 
        registry_state.peers[slot].in_use && 
        registry_state.peers[slot].device_id == device_id) {
        return slot;
    }
    
    // Hash collision - linear search (rare case)
    for (int i = 0; i < PEER_REGISTRY_MAX_PEERS; i++) {
        if (registry_state.peers[i].in_use && 
            registry_state.peers[i].device_id == device_id) {
            return i;
        }
    }
    return -1;
}

// Helper function to find peer slot by MAC address - O(n)
static int find_peer_slot_by_mac(const uint8_t *mac_addr)
{
    ESP_LOGI(TAG,"finding record");
    if (!mac_addr) {
        return -1;
    }

    for (int i = 0; i < PEER_REGISTRY_MAX_PEERS; i++) {
        if (registry_state.peers[i].in_use && 
            memcmp(registry_state.peers[i].mac_addr, mac_addr, 6) == 0) {
            return i;
        }
    }
    return -1;
}

// Helper function to find free slot
static int find_free_slot(void)
{
    for (int i = 0; i < PEER_REGISTRY_MAX_PEERS; i++) {
        if (!registry_state.peers[i].in_use) {
            return i;
        }
    }
    return -1;
}


esp_err_t peer_registry_deinit(void)
{
    if (!registry_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Clear all entries and hash table
    memset(registry_state.peers, 0, sizeof(registry_state.peers));
    for (int i = 0; i < PEER_REGISTRY_MAX_PEERS; i++) {
        registry_state.id_to_slot[i] = PEER_REGISTRY_MAX_PEERS;
    }
    registry_state.peer_count = 0;
    registry_state.initialized = false;

    ESP_LOGI(TAG, "Peer registry deinitialized");
    return ESP_OK;
}

esp_err_t peer_registry_add_peer(peer_device_id_t device_id, const uint8_t *mac_addr, const char *name)
{
    if (!registry_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (device_id == PEER_REGISTRY_INVALID_ID || !mac_addr) {
        ESP_LOGE(TAG, "Invalid parameters");
        return ESP_ERR_INVALID_ARG;
    }

    // Check if device ID already exists
    if (find_peer_slot_by_id(device_id) >= 0) {
        ESP_LOGW(TAG, "Peer with ID %u already exists", device_id);
        return ESP_ERR_INVALID_STATE;
    }

    // Check if MAC already exists
    if (find_peer_slot_by_mac(mac_addr) >= 0) {
        ESP_LOGW(TAG, "Peer with MAC " MACSTR " already exists", MAC2STR(mac_addr));
        return ESP_ERR_INVALID_STATE;
    }

    // Find free slot
    int slot = find_free_slot();
    if (slot < 0) {
        ESP_LOGE(TAG, "Registry full, cannot add peer");
        return ESP_ERR_NO_MEM;
    }

    // Add peer
    registry_state.peers[slot].device_id = device_id;
    memcpy(registry_state.peers[slot].mac_addr, mac_addr, 6);
    if (name) {
        strncpy(registry_state.peers[slot].name, name, PEER_REGISTRY_MAX_NAME_LEN - 1);
        registry_state.peers[slot].name[PEER_REGISTRY_MAX_NAME_LEN - 1] = '\0';
    } else {
        snprintf(registry_state.peers[slot].name, PEER_REGISTRY_MAX_NAME_LEN, "Device_%u", device_id);
    }
    registry_state.peers[slot].in_use = true;
    registry_state.peer_count++;

    // Update hash table
    uint16_t hash_slot = device_id_to_slot_hash(device_id);
    registry_state.id_to_slot[hash_slot] = slot;

    ESP_LOGI(TAG, "Added peer ID=%u ('%s') with MAC " MACSTR, 
             device_id, registry_state.peers[slot].name, MAC2STR(mac_addr));
    return ESP_OK;
}

esp_err_t peer_registry_remove_peer_by_id(peer_device_id_t device_id)
{
    if (!registry_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    int slot = find_peer_slot_by_id(device_id);
    if (slot < 0) {
        ESP_LOGW(TAG, "Peer with ID %u not found", device_id);
        return ESP_ERR_NOT_FOUND;
    }

    // Clear hash table entry
    uint16_t hash_slot = device_id_to_slot_hash(device_id);
    registry_state.id_to_slot[hash_slot] = PEER_REGISTRY_MAX_PEERS;

    // Clear entry
    ESP_LOGI(TAG, "Removed peer ID=%u ('%s')", device_id, registry_state.peers[slot].name);
    memset(&registry_state.peers[slot], 0, sizeof(peer_registry_entry_t));
    registry_state.peer_count--;

    return ESP_OK;
}

esp_err_t peer_registry_remove_peer_by_mac(const uint8_t *mac_addr)
{
    if (!registry_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!mac_addr) {
        return ESP_ERR_INVALID_ARG;
    }

    int slot = find_peer_slot_by_mac(mac_addr);
    if (slot < 0) {
        ESP_LOGW(TAG, "Peer with MAC " MACSTR " not found", MAC2STR(mac_addr));
        return ESP_ERR_NOT_FOUND;
    }

    peer_device_id_t device_id = registry_state.peers[slot].device_id;
    
    // Clear hash table entry
    uint16_t hash_slot = device_id_to_slot_hash(device_id);
    registry_state.id_to_slot[hash_slot] = PEER_REGISTRY_MAX_PEERS;

    ESP_LOGI(TAG, "Removed peer ID=%u ('%s') with MAC " MACSTR, 
             device_id, registry_state.peers[slot].name, MAC2STR(mac_addr));
    
    // Clear entry
    memset(&registry_state.peers[slot], 0, sizeof(peer_registry_entry_t));
    registry_state.peer_count--;

    return ESP_OK;
}

esp_err_t peer_registry_get_mac_by_id(peer_device_id_t device_id, uint8_t *mac_addr)
{
    if (!registry_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (device_id == PEER_REGISTRY_INVALID_ID || !mac_addr) {
        return ESP_ERR_INVALID_ARG;
    }

    int slot = find_peer_slot_by_id(device_id);
    if (slot < 0) {
        return ESP_ERR_NOT_FOUND;
    }

    memcpy(mac_addr, registry_state.peers[slot].mac_addr, 6);
    return ESP_OK;
}

esp_err_t peer_registry_get_id_by_mac(const uint8_t *mac_addr, peer_device_id_t *device_id)
{
    if (!registry_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!mac_addr || !device_id) {
        return ESP_ERR_INVALID_ARG;
    }

    int slot = find_peer_slot_by_mac(mac_addr);
    if (slot < 0) {
        return ESP_ERR_NOT_FOUND;
    }

    *device_id = registry_state.peers[slot].device_id;
    return ESP_OK;
}

esp_err_t peer_registry_get_name_by_id(peer_device_id_t device_id, char *name)
{
    if (!registry_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (device_id == PEER_REGISTRY_INVALID_ID || !name) {
        return ESP_ERR_INVALID_ARG;
    }

    int slot = find_peer_slot_by_id(device_id);
    if (slot < 0) {
        return ESP_ERR_NOT_FOUND;
    }

    strncpy(name, registry_state.peers[slot].name, PEER_REGISTRY_MAX_NAME_LEN);
    return ESP_OK;
}

bool peer_registry_exists_by_id(peer_device_id_t device_id)
{
    if (!registry_state.initialized) {
        return false;
    }

    return find_peer_slot_by_id(device_id) >= 0;
}

bool peer_registry_exists_by_mac(const uint8_t *mac_addr)
{

    ESP_LOGI(TAG,"checking record");
    if (!registry_state.initialized) {
        return false;
    }

    return find_peer_slot_by_mac(mac_addr) >= 0;
}

uint16_t peer_registry_get_count(void)
{
    if (!registry_state.initialized) {
        return 0;
    }

    return registry_state.peer_count;
}

uint16_t peer_registry_get_capacity(void)
{
    if (!registry_state.initialized) {
        return 0;
    }

    return registry_state.max_peers;
}

esp_err_t peer_registry_clear(void)
{
    if (!registry_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    memset(registry_state.peers, 0, sizeof(registry_state.peers));
    for (int i = 0; i < PEER_REGISTRY_MAX_PEERS; i++) {
        registry_state.id_to_slot[i] = PEER_REGISTRY_MAX_PEERS;
    }
    registry_state.peer_count = 0;

    ESP_LOGI(TAG, "Registry cleared");
    return ESP_OK;
}

esp_err_t peer_registry_get_all_peers(peer_registry_entry_t *entries, uint16_t max_entries, uint16_t *actual_count)
{
    if (!registry_state.initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!entries || !actual_count) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t count = 0;
    for (int i = 0; i < PEER_REGISTRY_MAX_PEERS && count < max_entries; i++) {
        if (registry_state.peers[i].in_use) {
            memcpy(&entries[count], &registry_state.peers[i], sizeof(peer_registry_entry_t));
            count++;
        }
    }

    *actual_count = count;
    return ESP_OK;
}




peer_registry_interface_t* peer_registry_init(const peer_registry_config_t *config)
{
    if (registry_state.initialized) {
        ESP_LOGW(TAG, "Already initialized");
        return NULL;
    }

    // Set configuration
    if (config && config->max_peers > 0 && config->max_peers <= PEER_REGISTRY_MAX_PEERS) {
        registry_state.max_peers = config->max_peers;
    } else {
        registry_state.max_peers = PEER_REGISTRY_MAX_PEERS;
    }

    // Initialize hash table with invalid slots
    for (int i = 0; i < PEER_REGISTRY_MAX_PEERS; i++) {
        registry_state.id_to_slot[i] = PEER_REGISTRY_MAX_PEERS;  // Invalid slot marker
    }

    // Clear all entries
    memset(registry_state.peers, 0, sizeof(registry_state.peers));
    registry_state.peer_count = 0;
    registry_state.initialized = true;
    registry_state.interface.peer_registry_exists_by_id=peer_registry_exists_by_id;
    registry_state.interface.peer_registry_exists_by_mac=peer_registry_exists_by_mac;
    //registry_state.interface.peer_registry_generate_id;
    registry_state.interface.peer_registry_get_count=peer_registry_get_count;
    registry_state.interface.peer_registry_add_peer=peer_registry_add_peer;
    
    

    ESP_LOGI(TAG, "Peer registry initialized with capacity %d", registry_state.max_peers);
    return &registry_state.interface;
}

/*peer_device_id_t peer_registry_generate_id(void)
{
    if (!registry_state.initialized) {
        return PEER_REGISTRY_INVALID_ID;
    }

    // Find the next available ID starting from 1
    for (peer_device_id_t id = 1; id <= UINT16_MAX; id++) {
        if (!peer_registry_exists_by_id(id)) {
            return id;
        }
    }

    return PEER_REGISTRY_INVALID_ID;  // All IDs are taken (very unlikely)
}*/