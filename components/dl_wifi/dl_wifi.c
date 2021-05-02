// DATA LAYER
// Source file of the wifi component.

// components
#include "./dl_wifi.h"

#include "../general/general.h"

// esp-idf
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_wifi.h"

// default values for wifi config. get them from sdkconfig.h
// which is made from the options in Kconfig.
#define DEFAULT_SSID CONFIG_APP_WIFI_SSID
#define DEFAULT_PWD CONFIG_APP_WIFI_PWD

#if CONFIG_APP_WIFI_ALL_CHANNEL_SCAN
#define DEFAULT_SCAN_METHOD WIFI_ALL_CHANNEL_SCAN
#elif CONFIG_APP_WIFI_FAST_SCAN
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#else
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#endif

// time to wait in seconds before reconnecting to wifi
// station
#define RECONNECT_TIME 10

static const char *TAG = "dl_wifi";

void dl_wifi_init() {
    // create a LwIP core task and initialize LwIP related
    // work, see `esp_netif.h`. formerly this was the
    // tcpip_adapter.
    log_status(TAG, esp_netif_init(), "esp_netif_init");

    // create a default wifi station
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    // create wifi init configuration object from default,
    // see `esp_wifi.h`
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    // init with config object
    log_status(TAG, esp_wifi_init(&cfg), "esp_wifi_init");

    // wifi contiguration object in station mode (conncect
    // to access point)
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = DEFAULT_SSID,
            .password = DEFAULT_PWD,
            .scan_method = DEFAULT_SCAN_METHOD},
    };

    // set mode to station mode which means it will connect
    // to another access point.
    log_status(TAG,
               esp_wifi_set_mode(WIFI_MODE_STA),
               "esp_wifi_set_mode station mode");

    // apply the wifi configuration object
    log_status(TAG,
               esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config),
               "esp_wifi_set_config default config with ssid and password");

    // start wifi component this post WIFI_EVENT_STA_START
    log_status(TAG,
               esp_wifi_start(),
               "esp_wifi_start");

    // connection is done in event handler
    // events are:
    // WIFI_EVENT_STA_START
    // WIFI_EVENT_STA_CONNECTED
    // WIFI_EVENT_STA_DISCONNECTED
    // IP_EVENT_STA_GOT_IP
    ESP_LOGI(TAG, "Finished init");
}

void dl_wifi_handler(void *arg,
                     esp_event_base_t base,
                     int32_t id,
                     void *data) {
    ESP_LOGV(TAG,
             "Handling event with base: %s - id: %d",
             base,
             id);

    if (base == WIFI_EVENT) {
        // handle the wifi events
        switch (id) {
            case WIFI_EVENT_STA_START:
                // this event is posted right after
                // `esp_wifi_start`. now you can connect to
                // the wifi network
                log_status(TAG,
                           esp_wifi_connect(),
                           "esp_wifi_connect");
                break;
            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Connected to network");
                // now the DHCP process will start
                // internally. nothing from the main task is
                // required. after successful DHCP process
                // the event IP_EVENT_STA_GOT_IP will be
                // posted
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                ESP_LOGW(TAG,
                         "Got disconnected, trying to reconnect in %ds",
                         RECONNECT_TIME);
                sntp_stop();

                vTaskDelay(RECONNECT_TIME * 1000 / portTICK_PERIOD_MS);
                log_status(TAG,
                           esp_wifi_connect(),
                           "esp_wifi_connect");
                break;

            default:
                // nothing
                break;
        }
    }
}
