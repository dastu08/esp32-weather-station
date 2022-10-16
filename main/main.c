// MAIN FILE

// components
#include "../components/al_bmp180/al_bmp180.h"
#include "../components/al_crypto/al_crypto.h"
#include "../components/al_weather_station/al_weather_station.h"
#include "../components/dl_wifi/dl_wifi.h"
#include "../components/general/general.h"
#include "../components/heartbeat/heartbeat.h"
#include "../components/pl_i2c/pl_i2c.h"
#include "../components/pl_udp/pl_udp.h"

// esp-idf
// #include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "string.h"

#define ENABLE_WEATHER_STATION
#define ENABLE_HEARTBEAT
#define ENABLE_WIFI
#define ENABLE_BMP180
#define ENABLE_CRYPTO

// period of the weather station measurements in seconds
#define MEASUREMENT_RATE 3*3600
// period of the heartbeat timer in seconds
#define HEARTBEAT_RATE 3600
// port for the udp communication
#define UDP_PORT 50000

// Tag for logging from this file
static const char* TAG = "user";

// application entry point
void app_main() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // log on app start as warning to make it visible
    ESP_LOGW(TAG, "Hello world!");

    esp_log_level_set("user", ESP_LOG_INFO);

    // create an event loop
    log_status(TAG,
               esp_event_loop_create_default(),
               "esp_event_loop_create_default");

#ifdef ENABLE_BMP180
    esp_log_level_set("pl_i2c", ESP_LOG_INFO);
    esp_log_level_set("al_bmp180", ESP_LOG_INFO);

    // init the I2C driver needed for the BMP180
    pl_i2c_init(GPIO_NUM_19, GPIO_NUM_18, 100000);
    // now init the bm180 itself
    al_bmp180_init();
#endif  // ENABLE_BMP180

#ifdef ENABLE_WIFI
    // show less infos from the internal wifi component
    esp_log_level_set("wifi", ESP_LOG_WARN);
    esp_log_level_set("dl_wifi", ESP_LOG_INFO);
    esp_log_level_set("pl_udp", ESP_LOG_INFO);

    // register the event handlers for any wifi or ip events
    // to the default event loop. need `esp_event.h` to
    // register the event handler.
    log_status(TAG,
               esp_event_handler_instance_register(WIFI_EVENT,
                                                   ESP_EVENT_ANY_ID,
                                                   &dl_wifi_handler,
                                                   NULL,
                                                   NULL),
               "register wifi events (any)");
    log_status(TAG,
               esp_event_handler_instance_register(IP_EVENT,
                                                   IP_EVENT_STA_GOT_IP,
                                                   &pl_udp_handler,
                                                   NULL,
                                                   NULL),
               "register ip event IP_EVENT_STA_GOT_IP");

    // init the udp component
    pl_udp_init(UDP_PORT);
    // init the wifi component
    dl_wifi_init();
#endif  // ENABLE_WIFI

#ifdef ENABLE_HEARTBEAT
    esp_log_level_set("heartbeat", ESP_LOG_INFO);

    // register the handler upon a heartbeat event which are
    // triggered by a timer
    log_status(TAG,
               esp_event_handler_instance_register(HEARTBEAT_EVENT,
                                                   HEARTBEAT_EVENT_SEND,
                                                   &heartbeat_handler,
                                                   NULL,
                                                   NULL),
               "register heartbeat event HEARTBEAT_EVENT_SEND_handler");

    // init and start the heartbeat timer
    heartbeat_set_period(HEARTBEAT_RATE);
    heartbeat_init();
    heartbeat_start();
#endif  // ENABLE_HEARTBEAT

#ifdef ENABLE_WEATHER_STATION
    esp_log_level_set("weather_station", ESP_LOG_INFO);

    // register the handler that handles incoming UDP
    // messages and thus initiates measurements
    log_status(TAG,
               esp_event_handler_instance_register(UDP_EVENT,
                                                   UDP_EVENT_RECEIVED,
                                                   &al_weather_station_handler,
                                                   NULL,
                                                   NULL),
               "register udp event UDP_EVENT_RECEIVED handler");

    // init and start the measurement timer
    al_weather_station_init();
    al_weather_station_start(MEASUREMENT_RATE);
#endif  // ENABLE_WEATHER_STATION

#ifdef ENABLE_CRYPTO
    esp_log_level_set("al_crypto", ESP_LOG_INFO);
    al_crypto_init();

#endif  // ENABLE_CRYPTO

    while (1) {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
