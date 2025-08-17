#include <stdio.h>
#include "espnow_discovery.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "unity.h"
#include "esp_log.h"
#include "esp_err.h"
static const char* TAG="test discovery";

static config_espnow_discovery config;
static espnow_discovery_interface_t discovery;
static discovery_whitelist_interface_t whitelist;
static discovery_timer_interface_t timer;



esp_err_t acknowledge_the_discovery(const uint8_t* mac_addr){
    ESP_LOGI(TAG, "ack : ok see you");

    return 0;

}

esp_err_t add_peer(const uint8_t* mac_addr){
    ESP_LOGI(TAG, "adding peer");
    return 0;

}

/*
void process_discovery_acknowledgement_callback(const uint8_t* mac_addr){

    ESP_LOGI(TAG, "yes discovered u");
    
}

void process_discovery_callback(const uint8_t* mac_addr){

    ESP_LOGI(TAG, "so it is u");
    
}*/


esp_err_t send_discovery(const uint8_t* mac_addr){

    ESP_LOGI(TAG, "hello any body there");
    return 0;
}


bool is_white_listed(const uint8_t* mac_addr){
    return true;
}

uint32_t get_current_time(){
    static uint32_t t=0;
    t+=100;
    ESP_LOGI(TAG,"current time %" PRIu32,t);
    return t;
}
esp_err_t start_timer(){

    ESP_LOGI(TAG,"timer started");
    return 0;
}

esp_err_t stop_timer(){

    ESP_LOGI(TAG,"timer stopped");
    return 0;
}


static void mysetUp(){


    
    discovery.acknowledge_the_discovery=acknowledge_the_discovery;
    discovery.add_peer=add_peer;
    
    //These two are assigned by the code to be tested
    //discovery.process_discovery_acknowledgement_callback=process_discovery_acknowledgement_callback;
    //discovery.process_discovery_callback=process_discovery_callback;
    discovery.send_discovery= send_discovery;
    whitelist.is_white_listed=is_white_listed;
    
    timer.get_current_time=get_current_time;
    timer.start_timer=start_timer;
    timer.stop_timer=stop_timer;
    config.discovery=&discovery;
    config.timer=&timer;
    config.whitelist=&whitelist;
    config.discovery_duration=1000;
    config.discovery_interval=100;
    discovery_service_init(&config);

}

TEST_CASE("Discovery","[Unit Test: Send]"){

    mysetUp();
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();
    vTaskDelay(pdMS_TO_TICKS(100));
    timer.discovery_timer_callback();


}

TEST_CASE("Discovery","[Unit Test: Discovery Arrived]"){

    mysetUp();
    uint8_t mac_addr[]={1,2,3,4,5,6};
    discovery.process_discovery_callback(mac_addr);

}

TEST_CASE("Discovery","[Unit Test: Discovery Ack arrived]"){

    mysetUp();
    uint8_t mac_addr[]={1,2,3,4,5,6};
    discovery.process_discovery_acknowledgement_callback(mac_addr);

}