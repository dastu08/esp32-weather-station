// APPLICATION LAYER
// Header file of the Weather Station component.

#ifndef _AL_WEATHER_STATION_H_
#define _AL_WEATHER_STATION_H_

#include "esp_event.h"
#include "esp_timer.h"

/** Initialize the measurement timer.

**Description**
    Set the esp_timer args with the callback and a name.
    Create the timer with `esp_timer_create`.
*/
void al_weather_station_init();

/** Start the measurement timer.

**Parameters**
    - period: 
        the period of the periodic timer in seconds

**Requirements**
    Initatilze the timer first with
    `al_weather_station_init`.

**Description**
    Start the timer from `al_weather_station_init` with the
    given period.
*/
void al_weather_station_start(uint64_t period);

/** Stop the measurement timer.

**Requirements**
    Initatilze the timer first with
    `al_weather_station_init`. The measurement timer must
    also be running.

**Description**
    Stop the timer from `al_weather_station_init`.
*/
void al_weather_station_stop();

void al_weather_station_handler(void* arg,
                                esp_event_base_t base,
                                int32_t id,
                                void* data);

#endif  // _AL_WEATHER_STATION_H_