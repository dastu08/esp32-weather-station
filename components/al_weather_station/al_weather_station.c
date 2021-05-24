// APPLICATION LAYER
// Source file of the Weather Station component.

#include "./al_weather_station.h"

#include <string.h>

#include "../al_bmp180/al_bmp180.h"
#include "../general/general.h"
#include "../heartbeat/heartbeat.h"
#include "../pl_udp/pl_udp.h"
#include "cJSON.h"

typedef enum {
    TEMPERATURE,
    PRESSURE
} quantity_type_t;

typedef enum {
    HEARTBEAT,
    HEARTBEAT_INTERVAL,
    MEASUREMENT_INTERVAL
} name_type_t;

static const char *TAG = "weather_station";

// handle to identify the timer
esp_timer_handle_t measurement_timer;

// convert a string to a quanity_type number
uint8_t string2quanity_type(char *string) {
    quantity_type_t quantity_type;

    if (0 == strcmp(string, "temperature")) {
        quantity_type = TEMPERATURE;
    } else if (0 == strcmp(string, "pressure")) {
        quantity_type = PRESSURE;
    }

    return quantity_type;
}

// convert a string to a name_type number
uint8_t string2name_type(char *string) {
    name_type_t name_type;

    if (0 == strcmp(string, "heartbeat")) {
        name_type = HEARTBEAT;
    } else if (0 == strcmp(string, "heartbeat_interval")) {
        name_type = HEARTBEAT_INTERVAL;
    } else if (0 == strcmp(string, "measurement_interval")) {
        name_type = MEASUREMENT_INTERVAL;
    }

    return name_type;
}

// do the measurement of the specified quantity
void make_measurement(char *quantity_string) {
    char time_buf[32];
    char tx_buffer[128];
    int32_t quantity_value;

    switch (string2quanity_type(quantity_string)) {
        case TEMPERATURE:
            quantity_value = al_bmp180_get_temperature();
            get_time(time_buf);

            sprintf(tx_buffer,
                    "{\"type\":\"response\",\"time\":\"%s\",\"temperature\": %.1f}",
                    time_buf, (float)quantity_value / 10);
            pl_udp_send(tx_buffer);
            break;

        case PRESSURE:
            quantity_value = al_bmp180_get_pressure(1);
            get_time(time_buf);

            sprintf(tx_buffer,
                    "{\"type\":\"response\",\"time\":\"%s\",\"pressure\": %.3f}",
                    time_buf, (float)quantity_value / 100);
            pl_udp_send(tx_buffer);
            break;

        default:
            pl_udp_send("{\"type\":\"error\"}");
            break;
    }
}

// set the variable to the given value of type string
void set_variable_string(char *name_string, char *value_string) {
    switch (string2name_type(name_string)) {
        case HEARTBEAT:
            if (0 == strcmp(value_string, "on")) {
                heartbeat_start();
            } else if (0 == strcmp(value_string, "off")) {
                heartbeat_stop();
            }
            break;

        default:
            break;
    }
}

// set the variable to the given value of type int
void set_variable_int(char *name_string, uint8_t value_int) {
    switch (string2name_type(name_string)) {
        case HEARTBEAT_INTERVAL:
            // restart the heartbeat timer with the new period
            heartbeat_stop();
            heartbeat_set_period(value_int);
            heartbeat_start();
            break;

        case MEASUREMENT_INTERVAL:
            // restart the measurement time with the new period
            al_weather_station_stop();
            al_weather_station_start(value_int);
            break;

        default:
            break;
    }
}

// function gets called when the timer runs out
void measurement_callback() {
    char time_buf[32];
    char tx_buffer[128];

    ESP_LOGD(TAG, "measurement started");

    int32_t t = al_bmp180_get_temperature();
    int32_t p = al_bmp180_get_pressure(3);
    get_time(time_buf);

    sprintf(tx_buffer,
            "{\"type\":\"measurement\",\"time\":\"%s\",\"temperature\": "
            "%.1f,\"pressure\": %.3f}",
            time_buf, (float)t / 10, (float)p / 100);

    pl_udp_send(tx_buffer);
}

void al_weather_station_init() {
    ESP_LOGI(TAG, "init started");

    // set up the timer stuff
    const esp_timer_create_args_t measurement_timer_args = {
        .callback = &measurement_callback, .name = "measurement"};

    log_status(TAG,
               esp_timer_create(&measurement_timer_args, &measurement_timer),
               "create measurement timer");

    ESP_LOGI(TAG, "init finished");
}

void al_weather_station_start(uint64_t period) {
    log_status(TAG,
               esp_timer_start_periodic(measurement_timer, period * 1000000),
               "starting measurement timer");
}

void al_weather_station_stop() {
    log_status(TAG,
               esp_timer_stop(measurement_timer),
               "stopped measurement timer");
}

void al_weather_station_handler(void *arg, esp_event_base_t base, int32_t id,
                                void *data) {
    cJSON *data_json = NULL;
    cJSON *data_type = NULL;

    // parse received data as json and evaluate the request
    data_json = cJSON_Parse(data);
    if (data_json != NULL) {
        // extract type to later distignuish get/set
        data_type = cJSON_GetObjectItemCaseSensitive(data_json, "type");
    } else {
        ESP_LOGW(TAG, "Couldn't parse JSON");
    }

    // handle get/set request
    if (data_type != NULL) {
        if (0 == strcmp(data_type->valuestring, "get")) {
            // extract the quanities that are specified in the get request
            cJSON *quantity = cJSON_GetObjectItemCaseSensitive(data_json, "quantity");

            if (quantity != NULL) {
                if (cJSON_IsArray(quantity)) {
                    uint8_t num_quanties = cJSON_GetArraySize(quantity);
                    ESP_LOGD(TAG, "GET request of %d quantities", num_quanties);
                    // if quanitiy is an array loop through the elements of the array
                    for (uint8_t k = 0; k < num_quanties; k++) {
                        make_measurement(cJSON_GetArrayItem(quantity, k)->valuestring);
                    }
                } else {
                    // handle the request if quantity is just a single element
                    ESP_LOGD(TAG, "GET request of quantity: %s", quantity->valuestring);
                    make_measurement(quantity->valuestring);
                }
            }
        } else if (0 == strcmp(data_type->valuestring, "set")) {
            // extract the name and value of the variable to set
            cJSON *name = cJSON_GetObjectItemCaseSensitive(data_json, "name");
            cJSON *value = cJSON_GetObjectItemCaseSensitive(data_json, "value");

            if ((name != NULL) & (value != NULL)) {
                if (cJSON_IsString(value)) {
                    ESP_LOGD(TAG, "SET request of variable: %s to %s", name->valuestring, value->valuestring);
                    set_variable_string(name->valuestring, value->valuestring);
                } else if (cJSON_IsNumber(value)) {
                    ESP_LOGD(TAG, "SET request of variable: %s to %d", name->valuestring, value->valueint);
                    set_variable_int(name->valuestring, value->valueint);
                }
            }
        }
    }
}