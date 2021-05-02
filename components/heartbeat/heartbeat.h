// MISCELLANEOUS
// Header file for the heartbeat component

#ifndef _HEARTBEAT_H_
#define _HEARTBEAT_H_

#include "esp_event.h"
#include "esp_timer.h"

// event base
ESP_EVENT_DECLARE_BASE(HEARTBEAT_EVENT);

// event id
typedef enum {
    HEARTBEAT_EVENT_SEND
} heartbeat_event_t;

/** Initialize the heartbeat timer.

**Description**
    Set the esp_timer args with the callback and a name.
    Create the timer with `esp_timer_create`.
*/
void heartbeat_init();

/** Start the heartbeat timer.

**Parameters**
    - period: 
        the period of the periodic timer in seconds

**Requirements**
    Initatilze the timer first with `heartbeat_init`.

**Description**
    Start the timer from `heartbeat_init` with the given
    period.
*/
void heartbeat_start(uint64_t period);

void heartbeat_handler(void* arg,
                       esp_event_base_t base,
                       int32_t id,
                       void* data);

#endif  // _HEARTBEAT_H_