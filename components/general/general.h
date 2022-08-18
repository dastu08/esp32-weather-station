// MISCELLANEOUS
// Header file for general functions

#ifndef _GENERAL_H_
#define _GENERAL_H_

// needed for `esp_err_t`
#include "esp_err.h"
// needed for ESP_LOG macros
#include "esp_log.h"

// define the global log level of all files that
// include this file
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// enable error code lookup
// #define CONFIG_ESP_ERR_TO_NAME_LOOKUP

/** Log status depending on error code.
 
**Parameters**
    - *tag: name of the logging component
    - status: error status structure
    - *msg: message or command to display

**Description**
    Log success or error message depending on status.
    Success is logged as VERBOSE and error is logged as
    WARNING.
*/
void log_status(const char *tag,
                esp_err_t status,
                const char *msg);

/** Get the current system time

**Requirements**
    Syncronize the time with sntp, needs wifi connection.

**Parameters**
    - buf: string buffer for the time output

**Description**
    Use `time` and `localtime` to get the time and then
    format it to YYYY-MM-DD HH:MM:SS.
*/
void get_time(char *buf);

/** Get a seed for random generator initializaiton

**Return**
    long number which changes after every function call

**Description**
    Use the current system time as a seed.
*/
long get_seed();

#endif  // _GENERAL_H_
