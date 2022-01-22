// APPLICATION LAYER
// Source file of the Weather Station component.

#include "./al_weather_station.h"

#include <string.h>

#include "../al_bmp180/al_bmp180.h"
#include "../al_crypto/al_crypto.h"
#include "../general/general.h"
#include "../heartbeat/heartbeat.h"
#include "../pl_udp/pl_udp.h"
#include "cJSON.h"

typedef enum {
    INVALID_QUANTITY,
    TEMPERATURE,
    PRESSURE
} quantity_type_t;

typedef enum {
    INVALID_NAME,
    HEARTBEAT,
    HEARTBEAT_INTERVAL,
    MEASUREMENT_INTERVAL
} name_type_t;

static const char *TAG = "weather_station";

// handle to identify the timer
esp_timer_handle_t measurement_timer;

// PRIVATE FUNCTIONS

/** Convert a string to a quanity_type number.

** Parameters**
    - string: 
        char pointer that points to the string to be
        converted

**Return**
    A numerical value of the enum `quantity_type_t` is
    returned depending on the input string. The standard
    value if no string is matched is `INVALID_QUANTITY`.

**Description**
    Use `strcmp` to check the input string against known 
    types.
*/
uint8_t string2quanity_type(char *string) {
    quantity_type_t quantity_type = INVALID_QUANTITY;

    if (0 == strcmp(string, "temperature")) {
        quantity_type = TEMPERATURE;
    } else if (0 == strcmp(string, "pressure")) {
        quantity_type = PRESSURE;
    }

    return quantity_type;
}

/** Convert a string to a name_type number.

** Parameters**
    - string: 
        char pointer that points to the string to be
        converted

**Return**
    A numerical value of the enum `quantity_name_t` is
    returned depending on the input string. The standard
    value if no string is matched is `INVALID_NAME`.

**Description**
    Use `strcmp` to check the input string against known 
    types.
*/
uint8_t string2name_type(char *string) {
    name_type_t name_type = INVALID_NAME;

    if (0 == strcmp(string, "heartbeat")) {
        name_type = HEARTBEAT;
    } else if (0 == strcmp(string, "heartbeat_interval")) {
        name_type = HEARTBEAT_INTERVAL;
    } else if (0 == strcmp(string, "measurement_interval")) {
        name_type = MEASUREMENT_INTERVAL;
    }

    return name_type;
}

/** Do the measurement of the specified quantity.

**Parameters**
    - quantity_string:
        char pointer to the string that can be converted to 
        a quantity type by `string2quantity_type`
    
**Description**
    Start issuing the measurment on the BMP180. This include
    the temperature and the pressure. Get the system time
    and send the result of the measurement together with the 
    time stamp via UDP. If the input string does not match
    send an error.
*/
void make_measurement(char *quantity_string) {
    char time_buf[32];
    char tx_buffer[256];
    int32_t quantity_value;

    switch (string2quanity_type(quantity_string)) {
        case TEMPERATURE:
            quantity_value = al_bmp180_get_temperature();
            get_time(time_buf);

            sprintf(tx_buffer,
                    "{\"type\":\"response\",\"time\":\"%s\",\"quantity\":"
                    "[{\"name\":\"temperature\",\"value\": %.1f,\"unit\":\"celsius\"}]}",
                    time_buf, (float)quantity_value / 10);
            mycryptoinit();
            pl_udp_send(tx_buffer);
            break;

        case PRESSURE:
            quantity_value = al_bmp180_get_pressure(1);
            get_time(time_buf);

            sprintf(tx_buffer,
                    "{\"type\":\"response\",\"time\":\"%s\",\"quantity\":"
                    "[{\"name\":\"pressure\",\"value\": %.3f,\"unit\":\"hPa\"}]}",
                    time_buf, (float)quantity_value / 100);
            pl_udp_send(tx_buffer);
            break;

        default:
            pl_udp_send("{\"type\":\"error\"}");
            break;
    }
}

/** Set the variable to the given value of type string.

**Parameters**
    - name_string:
        string that contains the name of the variable to be 
        set. This must be convertable by `string2name_type`
    - value_string:
        string containing the value for the set variable

**Description**
    Convert name_string to a `name_type_t`. Handle the cases
    from there. Turn the heartbeat `on` or `off` with this.
*/
void set_variable_string(char *name_string,
                         char *value_string) {
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

/** Set the variable to the given value of type int.

**Parameters**
    - name_string:
        string that contains the name of the variable to be 
        set. This must be convertable by `string2name_type`
    - value_int:
        integer containing the value for the set variable

**Description**
    Convert name_string to a `name_type_t`. Handle the cases
    from there. Set the time intervals of the heartbeat and
    measurement timers with this.
*/
void set_variable_int(char *name_string,
                      uint64_t value_int) {
    switch (string2name_type(name_string)) {
        case HEARTBEAT_INTERVAL:
            // restart the heartbeat timer with the new
            // period
            heartbeat_stop();
            heartbeat_set_period(value_int);
            heartbeat_start();
            break;

        case MEASUREMENT_INTERVAL:
            // restart the measurement time with the new
            // period
            al_weather_station_stop();
            al_weather_station_start(value_int);
            break;

        default:
            break;
    }
}

/** Function gets called when the timer runs out.

**Description**
    Perform the measurments of temperature and pressure. 
    Save the system time of the measurment time point. Send
    the results together with the time tag via UDP.
*/
void measurement_callback() {
    char time_buf[32];
    char tx_buffer[256];

    ESP_LOGD(TAG, "measurement started");

    int32_t t = al_bmp180_get_temperature();
    int32_t p = al_bmp180_get_pressure(3);
    get_time(time_buf);

    sprintf(tx_buffer,
            "{\"type\":\"measurement\",\"time\":\"%s\",\"quantity\":["
            "{\"name\":\"temperature\",\"value\": %.1f,\"unit\":\"celsius\"},"
            "{\"name\":\"pressure\",\"value\": %.3f,\"unit\":\"hPa\"}]}",
            time_buf, (float)t / 10, (float)p / 100);

    pl_udp_send(tx_buffer);
}

// PUBLIC FUNCTIONS

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