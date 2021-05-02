// APPLICATION LAYER
// Source file of the Weather Station component.

#include <string.h>

#include "./al_weather_station.h"

#include "../al_bmp180/al_bmp180.h"
#include "../general/general.h"
#include "../pl_udp/pl_udp.h"

#include "cJSON.h"

static const char* TAG = "weather_station";

// handle to identify the timer
esp_timer_handle_t measurement_timer;

// function gets called when the timer runs out
void measurement_callback() {
    char time_buf[32];
    char tx_buffer[128];

    ESP_LOGD(TAG, "measurement started");

    int32_t t = al_bmp180_get_temperature();
    int32_t p = al_bmp180_get_pressure(3);
    get_time(time_buf);

    sprintf(tx_buffer,
            "{\"type\":\"measurement\",\"time\":\"%s\",\"temperature\": %.1f,\"pressure\": %.3f}",
            time_buf,
            (float)t / 10,
            (float)p / 100);

    pl_udp_send(tx_buffer);
}

void al_weather_station_init() {
    ESP_LOGI(TAG, "init started");

    // set up the timer stuff
    const esp_timer_create_args_t measurement_timer_args = {
        .callback = &measurement_callback,
        .name = "measurement"};

    log_status(TAG,
               esp_timer_create(&measurement_timer_args,
                                &measurement_timer),
               "create measurement timer");

    ESP_LOGI(TAG, "init finished");
}

void al_weather_station_start(uint64_t period) {
    log_status(TAG,
               esp_timer_start_periodic(measurement_timer,
                                        period * 1000000),
               "starting measurement timer");
}

void al_weather_station_handler(void* arg,
                                esp_event_base_t base,
                                int32_t id,
                                void* data) {
    char time_buf[32];
    char tx_buffer[128];

    cJSON *json = cJSON_Parse(data);

    if (0 == strcmp(data, "{\"type\":\"get\",\"what\":\"temperature\"}")) {
        ESP_LOGD(TAG, "get temperature");
        int32_t t = al_bmp180_get_temperature();
        get_time(time_buf);

        sprintf(tx_buffer, "{\"type\":\"response\",\"time\":\"%s\",\"temperature\": %.1f}", time_buf, (float)t / 10);
        pl_udp_send(tx_buffer);

    } else if (0 == strcmp(data, "{\"type\":\"get\",\"what\":\"pressure\"}")) {
        ESP_LOGD(TAG, "get pressure");
        int32_t p = al_bmp180_get_pressure(1);
        get_time(time_buf);

        sprintf(tx_buffer, "{\"type\":\"response\",\"time\":\"%s\",\"pressure\": %.3f}", time_buf, (float)p / 100);
        pl_udp_send(tx_buffer);
    } else {
        pl_udp_send("{\"type\":\"error\"}");
    }
}