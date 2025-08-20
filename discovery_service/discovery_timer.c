#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "discovery_timer.h"

static const char* TAG="discovery timer";

static struct{

    uint32_t duration;
    TimerHandle_t timer;    
    //discovery_timer_callback cb;    
    discovery_timer_implementation_t timer_interface;

}discovery_timer={0};   


//Call the registered callback
static void discovery_timer_callback_handler(TimerHandle_t timer){
    discovery_timer.timer_interface.callback_handler();
}


static esp_err_t start_timer(){
   
    if(xTimerStart(discovery_timer.timer, 0)==pdFAIL)
        return ESP_FAIL;
    return ESP_OK;
}


static esp_err_t stop_timer(){
   
    if(xTimerStop(discovery_timer.timer, 0)==pdFAIL)
        return ESP_FAIL;
    return ESP_OK;
}

static uint32_t get_current_time(){

    TickType_t current_ticks = xTaskGetTickCount();
    uint32_t current_ms = pdTICKS_TO_MS(current_ticks);
    return current_ms;
}

/*static void register_callback(discovery_timer_callback callback){

    discovery_timer.cb=callback;

}*/






discovery_timer_implementation_t* timer_create(uint32_t duration_ms){

    discovery_timer.timer = xTimerCreate(
        "discovery_timer",
        pdMS_TO_TICKS(duration_ms),
        pdTRUE,  // Auto-reload
        NULL,
        discovery_timer_callback_handler
    );

    if (!discovery_timer.timer) {
        ESP_LOGI(TAG, "Failed to create discovery timer");
        
        return NULL;
    }
    ESP_LOGI(TAG,"timer done before interface assign");
    discovery_timer.timer_interface.methods.start_timer=start_timer;
    discovery_timer.timer_interface.methods.stop_timer=stop_timer;
    //discovery_timer.timer_interface.register_callback=register_callback;
    discovery_timer.timer_interface.methods.get_current_time=get_current_time;
    ESP_LOGI(TAG,"timer done");
    return &discovery_timer.timer_interface;

}