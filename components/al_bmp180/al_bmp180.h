// / APPLICATION LAYER
// Headder file of the BMP180 component.

#ifndef _AL_BMP180_H_
#define _AL_BMP180_H_

#include "esp_err.h"

/** Initialize the BMP180.

**Return**
    - err: 
        success or fail if the sensor returns the correct id

**Description**
    Clear all calibration parameter. Calls
    `al_bmp180_get_calib_param`. Log the parameter to the
    console.
*/
esp_err_t al_bmp180_init();

/** Get the real temperature.

**Requirement**
    Initialize the BMP180 component with `al_bmp180_init`.

**Return**
    - t: temperature in units of 0.1 celsius

**Description**
    Calls `al_bmp180_get_ut` and does the temperature
    conversion. Log the temperature to the console in debug
    mode.
*/
int32_t al_bmp180_get_temperature();

/** Get the real pressure.

**Requirement**
    Initialize the BMP180 component with `al_bmp180_init`.
    Call `al_bmp180_get_temperature` before to get the value
    of `b5`.

**Parameters**
    - oss: 
        oversampling setting, possible values from 0-3 see
        `oss` in `al_bmp_180_get_ut`

**Return*
    - p: pressure in units of Pa

**Description**
    Calls `al_bmp180_get_up` and does the pressure
    conversion. Log the pressure to the console in debug
    mode.
*/
int32_t al_bmp180_get_pressure(uint8_t oss);

#endif  // _AL_BMP180_H_
