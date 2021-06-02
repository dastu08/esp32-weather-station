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


/** Set the rate for the heartbeat.

**Parameters**
    - period:
        the period of the periodic timer in seconds

**Description**
    Set the internal value for the heartbeat rate.
*/
void heartbeat_set_period(uint8_t period);

/** Start the heartbeat timer.

**Requirements**
    Initatilze the timer first with `heartbeat_init`.

**Description**
    Start the timer from `heartbeat_init` with the period
    set by `heartbeat_set_period`.
*/
void heartbeat_start();


/** Stops the heartbeat timer.

**Requirements**
    Initatilze the timer first with `heartbeat_init`.
    Call only when the timer is already running. 

**Description**
    Stop the running timer from `heartbeat_init`.
*/
void heartbeat_stop();

/** Event handler for heartbeat events.

**Parameters**
    - *arg : pointer to arguments of the event
    - base : event base
    - id : event id
    - *data : pointer to event data

**Requirements**
    Handler musst be registered for base `HEARTBEAT_EVENT` 
    and id `HEARTBEAT_EVENT_SEND` on the default event loop.

**Description**
    Send a heartbeat JSON object via UDP.
*/
void heartbeat_handler(void* arg,
                       esp_event_base_t base,
                       int32_t id,
                       void* data);

#endif  // _HEARTBEAT_H_