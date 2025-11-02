idf_component_register(SRCS esp_now_transport.c
                        INCLUDE_DIRS .
                        PRIV_REQUIRES nvs_flash esp_event esp_netif esp_wifi
                        REQUIRES event-adapter)