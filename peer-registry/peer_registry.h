// File: components/peer_registry/include/peer_registry.h

#ifndef PEER_REGISTRY_H
#define PEER_REGISTRY_H

#include "esp_err.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of peers that can be stored in the registry
#define PEER_REGISTRY_MAX_PEERS     32

// Maximum length of optional name string (including null terminator)
#define PEER_REGISTRY_MAX_NAME_LEN  32

// Invalid device ID constant
#define PEER_REGISTRY_INVALID_ID    0

/**
 * @brief Device ID type - use uint16_t for fast comparisons
 */
typedef uint16_t peer_device_id_t;

/**
 * @brief Peer registry entry structure
 */
typedef struct {
    peer_device_id_t device_id;                        ///< Unique device ID (integer, fast lookup)
    uint8_t mac_addr[6];                               ///< MAC address of the peer
    char name[PEER_REGISTRY_MAX_NAME_LEN];             ///< Optional human-readable name
    bool in_use;                                       ///< Whether this entry is active
} peer_registry_entry_t;

/**
 * @brief Peer registry configuration
 */
typedef struct {
    uint16_t max_peers;     ///< Maximum number of peers to support (must be <= PEER_REGISTRY_MAX_PEERS)
} peer_registry_config_t;


typedef struct{
    /**
     * @brief Deinitialize the peer registry
     * 
     * @return ESP_OK on success
     */
    esp_err_t (*peer_registry_deinit)(void);

    /**
     * @brief Add a peer to the registry
     * 
     * @param device_id Unique device ID (must be non-zero)
     * @param mac_addr MAC address of the peer
     * @param name Optional human-readable name )(can be NULL)
     * @return ESP_OK on success
     *         ESP_ERR_INVALID_ARG if parameters are invalid
     *         ESP_ERR_NO_MEM if registry is full
     *         ESP_ERR_INVALID_STATE if already exists
     */
    esp_err_t (*peer_registry_add_peer)(peer_device_id_t device_id, const uint8_t *mac_addr, const char *name);

    /**
     * @brief Remove a peer from the registry by device ID
     * 
     * @param device_id Device ID of the peer to remove
     * @return ESP_OK on success
     *         ESP_ERR_NOT_FOUND if peer not found
     */
    esp_err_t (*peer_registry_remove_peer_by_id)(peer_device_id_t device_id);

    /**
     * @brief Remove a peer from the registry by MAC address
     * 
     * @param mac_addr MAC address of the peer to remove
     * @return ESP_OK on success
     *         ESP_ERR_NOT_FOUND if peer not found
     */
    esp_err_t (*peer_registry_remove_peer_by_mac)(const uint8_t *mac_addr);

    /**
     * @brief Find MAC address by device ID - FAST O(1) lookup using direct indexing
     * 
     * @param device_id Device ID to search for
     * @param mac_addr Buffer to store the found MAC address (6 bytes)
     * @return ESP_OK on success
     *         ESP_ERR_NOT_FOUND if peer not found
     *         ESP_ERR_INVALID_ARG if parameters are invalid
     */
    esp_err_t (*peer_registry_get_mac_by_id)(peer_device_id_t device_id, uint8_t *mac_addr);

    /**
     * @brief Find device ID by MAC address - O(n) lookup
     * 
     * @param mac_addr MAC address to search for
     * @param device_id Pointer to store the found device ID
     * @return ESP_OK on success
     *         ESP_ERR_NOT_FOUND if peer not found
     *         ESP_ERR_INVALID_ARG if parameters are invalid
     */
    esp_err_t (*peer_registry_get_id_by_mac)(const uint8_t *mac_addr, peer_device_id_t *device_id);

    /**
     * @brief Get peer name by device ID
     * 
     * @param device_id Device ID to search for
     * @param name Buffer to store the name (must be at least PEER_REGISTRY_MAX_NAME_LEN bytes)
     * @return ESP_OK on success
     *         ESP_ERR_NOT_FOUND if peer not found
     *         ESP_ERR_INVALID_ARG if parameters are invalid
     */
    esp_err_t (*peer_registry_get_name_by_id)(peer_device_id_t device_id, char *name);

    /**
     * @brief Check if a peer exists by device ID - FAST O(1) lookup
     * 
     * @param device_id Device ID to check
     * @return true if peer exists, false otherwise
     */
    bool (*peer_registry_exists_by_id)(peer_device_id_t device_id);

    /**
     * @brief Check if a peer exists by MAC address - O(n) lookup
     * 
     * @param mac_addr MAC address to check
     * @return true if peer exists, false otherwise
     */
    bool (*peer_registry_exists_by_mac)(const uint8_t *mac_addr);

    /**
     * @brief Get the number of peers currently in the registry
     * 
     * @return Number of active peers
     */
    uint16_t (*peer_registry_get_count)(void);

    /**
     * @brief Get the maximum number of peers that can be stored
     * 
     * @return Maximum peer capacity
     */
    uint16_t (*peer_registry_get_capacity)(void);

    /**
     * @brief Clear all peers from the registry
     * 
     * @return ESP_OK on success
     */
    esp_err_t (*peer_registry_clear)(void);

    /**
     * @brief Get all peers in the registry
     * 
     * @param entries Buffer to store peer entries
     * @param max_entries Maximum number of entries that fit in the buffer
     * @param actual_count Pointer to store the actual number of entries returned
     * @return ESP_OK on success
     *         ESP_ERR_INVALID_ARG if parameters are invalid
     */
    esp_err_t (*peer_registry_get_all_peers)(peer_registry_entry_t *entries, uint16_t max_entries, uint16_t *actual_count);

    /**
     * @brief Generate a unique device ID automatically
     * 
     * This function finds the next available device ID starting from 1.
     * Useful when you don't want to manually assign IDs.
     * 
     * @return Next available device ID, or PEER_REGISTRY_INVALID_ID if registry is full
     */
    peer_device_id_t (*peer_registry_generate_id)(void);


    /**
     * @brief Initialize the peer registry
     * 
     * @param config Configuration structure )(can be NULL to use defaults)
     * @return ESP_OK on success
     */
}peer_registry_interface_t;

peer_registry_interface_t* peer_registry_init(const peer_registry_config_t *config);


#ifdef __cplusplus
}
#endif

#endif // PEER_REGISTRY_H