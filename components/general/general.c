// MISCELLANEOUS
// Source file for general functions

#include "./general.h"

#include <string.h>
#include <time.h>

void log_status(const char *tag,
                esp_err_t status,
                const char *msg) {
    // check status code
    if (status != ESP_OK) {
        // log on error as warning with status name and
        // status number
        ESP_LOGW(tag,
                 "failed %s: %s - 0x%x",
                 msg,
                 esp_err_to_name(status),
                 status);
    } else {
        // log if successful as verbose
        ESP_LOGV(tag, "success: %s", msg);
    }
}

void get_time(char *buf) {
    time_t epoch;
    struct tm date;

    // get current time
    time(&epoch);
    // set time zone
    setenv("TZ", "CET-1", 1);
    tzset();
    // get time in time zone format
    // localtime_r(&epoch, &date);
    // get time in UTC
    gmtime_r(&epoch, &date);
    // convert time to string
    sprintf(buf,
            "%04d-%02d-%02dT%02d:%02d:%02dZ",
            date.tm_year + 1900,
            date.tm_mon + 1,
            date.tm_mday,
            date.tm_hour,
            date.tm_min,
            date.tm_sec);
}

long get_seed() {
   time_t epoch; 
   // get current time
    time(&epoch);
    // convert to long as seed
    return (long)epoch;
    
}
