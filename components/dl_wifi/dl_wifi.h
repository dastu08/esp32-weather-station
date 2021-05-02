// DATA LAYER
// Header file of the wifi component.

#ifndef _DL_WIFI_H_
#define _DL_WIFI_H_

// needed for esp_event_base_t structure
#include "esp_event.h"

/** Init the wifi component.

**Requirements**
    Default event loop must exist, e.g. call
    `esp_event_loop_create_default()`. The event handler
    must be registered for WIFI_EVENT in the default event
    loop. 

** Description**
    Initialize the netif. Create a netif default wifi
    station. Call `esp_wifi_init` with default config
    object. Configure the wifi driver with password and ssid
    as given in menuconfig. Set to station mode and start.
    Wifi is ready to use when the event IP_EVENT_GOT_IP is
    thrown.
*/
void dl_wifi_init();

/** Event handler for wifi events.

**Parameters**
    - *arg : arguments of throw event function  
    - base : event base  
    - id : event id  
    - *data : pointer to event data  

**Description** 
    The handler is called when an event the handler is
    registered for is thrown. The base, id and data of the
    event are given as arguments to the handler. WiFi Events
    are: On WIFI_EVENT_STA_START connect to wifi network, on
    WIFI_EVENT_STA_CONNECTED do nothing and on
    WIFI_EVENT_STA_DISCONNECTED reconnect. Ip event is: On
    IP_EVENT_STA_GOT_IP log ip address.  
*/
void dl_wifi_handler(void *arg,
                     esp_event_base_t base,
                     int32_t id,
                     void *data);

#endif  // _DL_WIFI_H_