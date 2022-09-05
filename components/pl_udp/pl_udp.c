// PROTOCOL LAYER
// Source file of the UDP component.

// components
#include "./pl_udp.h"

#include "../al_crypto/al_crypto.h"
#include "../general/general.h"

// esp-idf
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "freertos/task.h"
#include "lwip/def.h"
#include "lwip/err.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

ESP_EVENT_DEFINE_BASE(UDP_EVENT);

// Socket file descriptor which will be the output of
// `socket(...)`.
int sock = -1;

// Socket structure for receiving and sending, see
// `lwip/sockets.h`
struct sockaddr_in rx_addr;
struct sockaddr_in tx_addr;
// Length of receiving socket structure from
// `sizeof(rx_addr)`.
socklen_t rx_addr_len;
socklen_t tx_addr_len;

// String buffer for incoming messages (127 characters).
char rx_buffer[128];
// String buffer for composing sending messages (127
// characters).
char tx_buffer[128];
// Buffer for ip address.
char ip_addr[128];

// Tag for logging from this component.
const char *TAG = "pl_udp";

// Flag checked by send and receive to see if udp is ready
// to use.
bool udp_ready = false;

void pl_udp_init(int port) {
    ESP_LOGI(TAG, "init starting");

    // reset upd ready flag
    udp_ready = false;

    // set ip family to IPv4
    tx_addr.sin_family = AF_INET;
    // broadcast ip address
    tx_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    // set the port
    tx_addr.sin_port = htons(port);
    // get the length/size of the socket structure
    tx_addr_len = sizeof(tx_addr);

    // set ip family to IPv4
    rx_addr.sin_family = AF_INET;
    // receive from any ip address
    rx_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // set the port
    rx_addr.sin_port = htons(port);
    // get the length/size of the socket structure
    rx_addr_len = sizeof(rx_addr);

    // see documentation for `htons()` and `htonl()`
    // https://pubs.opengroup.org/onlinepubs/007908799/xns/htonl.html
    // they are defined in `lwip/def.h`

    // Create and bind the sockets in the event handler. Use
    // sendto() or recvfrom() to transmitt or receive from
    // socket.
    ESP_LOGI(TAG, "init finished");
}

void pl_udp_handler(void *arg,
                    esp_event_base_t base,
                    int32_t id,
                    void *data) {
    int err;

    if (base == IP_EVENT) {
        switch (id) {
            case IP_EVENT_STA_GOT_IP:

                // create IPv4 socket and get file
                // descriptor
                // domain: AF_INET : IPv4
                // type: SOCK_DGRAM : datagram sockets
                // protocol : IPPROTO_UDP
                sock = socket(AF_INET,
                              SOCK_DGRAM,
                              IPPROTO_UDP);

                if (sock < 0) {
                    ESP_LOGW(TAG,
                             "udp socket does not exist");
                } else {
                    ESP_LOGV(TAG, "created udp socket");

                    // bind socket to receiving port
                    err = bind(sock,
                               (struct sockaddr *)&rx_addr,
                               rx_addr_len);

                    if (err < 0) {
                        ESP_LOGE(TAG,
                                 "unable to bind socket to port %d",
                                 ntohs(rx_addr.sin_port));
                    } else {
                        ESP_LOGD(TAG,
                                 "bound socket to port %d",
                                 ntohs(rx_addr.sin_port));

                        udp_ready = true;

                        // get time syncronization
                        sntp_setoperatingmode(SNTP_OPMODE_POLL);
                        sntp_setservername(0, "pool.ntp.org");
                        sntp_init();
                        ESP_LOGV(TAG, "starting sntp_init");
                    }
                }
                break;

            default:
                // nothing
                break;
        }

        // byte_t in[32] = "{\"type\":\"hello world\"}";
        // byte_t out[32 + 16];
        // al_crypto_encrypt(in, out);
        // pl_udp_send((char *)out);
        pl_udp_send("{\"type\":\"hello world\"}");

        // start listening
        xTaskCreate(pl_udp_receive,
                    "udp-receive",
                    4096,
                    NULL,
                    1,
                    NULL);
    }
}

void pl_udp_send(const char *msg) {
    const int ciphertext_length = 128 + 16;
    int msg_length = strlen(msg);
    byte_t ciphertext[ciphertext_length];

    if (msg_length > (ciphertext_length - 16)) {
        ESP_LOGW(TAG,
                 "cannot send a message of length %d bytes, maximum is 64 bytes. Aborting sending!",
                 msg_length);
        return;
    } else {
        ESP_LOGV(TAG, "plain message: %s", msg);
    }
    al_crypto_encrypt((byte_t *)msg, ciphertext);

    // check if socket was created
    if (sock >= 0 && udp_ready == true) {
        // send message via socket
        int err = sendto(sock,
                         ciphertext,
                         ciphertext_length,
                         0,
                         (struct sockaddr *)&tx_addr,
                         tx_addr_len);

        if (err < 0) {
            ESP_LOGE(TAG,
                     "unable to send message error %d",
                     err);
        } else {
            ESP_LOGD(TAG,
                     "<< %s:%d (%d bytes, %d words)",
                     inet_ntoa(tx_addr.sin_addr.s_addr),
                     ntohs(tx_addr.sin_port),
                     ciphertext_length,
                     ciphertext_length / 16);
        }
    }
}

void pl_udp_receive() {
    byte_t plaintext[128];
    // listening loop to start this function as a task
    while (1) {
        // check if socket was created
        if (sock >= 0 && udp_ready == true) {
            // receive message from bound socket and save in
            // rx_buffer
            int len = recvfrom(sock,
                               rx_buffer,
                               sizeof(rx_buffer) - 1,
                               0,
                               (struct sockaddr *)&rx_addr,
                               &rx_addr_len);

            if (len < 0) {
                ESP_LOGW(TAG,
                         "unable to receive message error %d",
                         len);
            } else {
                // null terminate buffer
                rx_buffer[len] = 0;

                // get ip address of sender in buffer ip_addr
                inet_ntoa_r(((struct sockaddr_in *)&rx_addr)->sin_addr.s_addr,
                            ip_addr,
                            sizeof(ip_addr) - 1);

                // print message
                ESP_LOGV(TAG,
                         ">> %s:%d (%d bytes, %d words)",
                         ip_addr,
                         ntohs(rx_addr.sin_port),
                         len,
                         len / 16);

                // TODO decrypt
                al_crypto_decrypt((byte_t *)rx_buffer, plaintext);

                esp_event_post(UDP_EVENT,
                               UDP_EVENT_RECEIVED,
                               plaintext,
                               sizeof(plaintext),
                               portMAX_DELAY);
            }
        }
    }
}