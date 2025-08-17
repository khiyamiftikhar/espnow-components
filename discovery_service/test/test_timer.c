#include <stdio.h>
#include "discovery_timer.h"
#include "unity.h"
#include "esp_log.h"
#include "esp_err.h"
#include <inttypes.h>
static const char* TAG="test timer";

static discovery_timer_interface_t* timer;



static void timer_callback(){

    ESP_LOGI(TAG,"timer elapsed");
    
}


void setUp(){

    timer=timer_create(2000);
    timer->discovery_timer_callback=timer_callback;

    
    
}


TEST_CASE("Timer","[Unit Test: Start]"){

    timer->start_timer();

}

TEST_CASE("Timer","[Unit Test: get current time]"){

    ESP_LOGI(TAG,"current time %"PRIu32,timer->get_current_time());

}