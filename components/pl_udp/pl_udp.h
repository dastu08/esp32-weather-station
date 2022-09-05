// PROTOCOL LAYER
// Header file of the UDP component.

#ifndef _PL_UDP_H_
#define _PL_UDP_H_

#include "esp_event.h"

// event base
ESP_EVENT_DECLARE_BASE(UDP_EVENT);

// event id
typedef enum {
    UDP_EVENT_RECEIVED
} udp_event_t;

/** Init of UDP component.

**Parameter**
    - port : listening/sending port

**Description** 
    Fill the values of the socket structures. Use
    `INADDR_BROADCAST` for sending ip address on port
    `port`. And listen from any ip address on port
    `port`.
*/
void pl_udp_init(int port);

/** Event handler to set up the UDP socket.

**Parameters**
    - *arg : pointer to arguments of the event
    - base : event base
    - id : event id
    - *data : pointer to event data

**Requirements**
    Handler must be registered for base `IP_EVENT` and id
    `IP_EVENT_STA_GOT_IP` on the default event loop. Before
    the wifi starts call `pl_udp_init()`.

**Description**
    For event `IP_EVENT_STA_GOT_IP` the receiving UDP socket
    will be created and bound to the listening port. After
    successful binding set `udp_ready` flag.
*/
void pl_udp_handler(void* arg,
                    esp_event_base_t base,
                    int32_t id,
                    void* data);

/** Send data via UDP encrypted.

**Parameters**
    - *msg : Message to send.

**Requirements**
    UDP must be init and flag `udp_ready` has to be true.
    Encryption must be initialized. The length of `msg` has
    to be less than the buffer length.

**Description**
    Encrypt the message. Send UDP message via socket and 
    `sendto()` to ip address set in `pl_udp_init()`.
*/
void pl_udp_send(const char* msg);

/** Receive encrypted data via UDP and print to log.

**Requirements**
    UDP must be init and flag `udp_read` true. The length
    of the incoming UDP message must be a multiple of 16.
    Maximum length is given by the buffer length.

**Description**
    Run loop for ever in a task.
    Receive bytes via socket and `recfrom()` from ip address
    set in `pl_udp_init()`. Decrypt the message. Log message 
    to esp log output as VERBOSE. Post a new udp event with 
    id UDP_EVENT_RECEIVED with the message as the data.
*/
void pl_udp_receive();

#endif  // _PL_UDP_H_