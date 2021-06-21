// MISCELLANEOUS
// Source file for the heartbeat component

#include "./heartbeat.h"

#include "../general/general.h"
#include "../pl_udp/pl_udp.h"

static const char* TAG = "heartbeat";

// heartbeat period in seconds
static uint64_t heartbeat_period = 120;

ESP_EVENT_DEFINE_BASE(HEARTBEAT_EVENT);

// handle to identify the timer
esp_timer_handle_t heartbeat_timer;

// function gets called when the timer runs out
void heartbeat_callback() {
    ESP_LOGD(TAG, "Heartbeat!");
    // post a heartbeat event
    esp_event_post(HEARTBEAT_EVENT,
                   HEARTBEAT_EVENT_SEND,
                   NULL,
                   0,
                   portMAX_DELAY);
}

void heartbeat_set_period(uint64_t period) {
    heartbeat_period = period;
}

void heartbeat_init() {
    // set up the timer stuff
    const esp_timer_create_args_t heartbeat_timer_args = {
        .callback = &heartbeat_callback,
        .name = "heartbeat"};

    log_status(TAG,
               esp_timer_create(&heartbeat_timer_args,
                                &heartbeat_timer),
               "create heartbeat timer");

    // setup the event stuff
}

void heartbeat_start() {
    log_status(TAG,
               esp_timer_start_periodic(heartbeat_timer,
                                        heartbeat_period * 1000000),
               "starting heartbeat timer");
}

void heartbeat_stop() {
    log_status(TAG,
               esp_timer_stop(heartbeat_timer),
               "stopped heartbeat timer");
}

void heartbeat_handler(void* arg,
                       esp_event_base_t base,
                       int32_t id,
                       void* data) {
    char time_buf[32];
    char msg[128];

    if (base == HEARTBEAT_EVENT) {
        switch (id) {
            case HEARTBEAT_EVENT_SEND:
                get_time(time_buf);
                sprintf(msg, "{\"type\":\"heartbeat\",\"time\":\"%s\"}", time_buf);

                ESP_LOGV(TAG, "%s", msg);

                // send the heartbeat via UDP
                pl_udp_send(msg);
                break;

            default:
                // nothing
                break;
        }
    }
}