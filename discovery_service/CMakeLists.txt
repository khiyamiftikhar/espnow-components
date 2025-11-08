idf_component_register(SRCS espnow_discovery.c discovery_timer.c
                        INCLUDE_DIRS .
                        PRIV_REQUIRES esp_timer 
                        
                        REQUIRES event-adapter database-interface common-interface
                        )