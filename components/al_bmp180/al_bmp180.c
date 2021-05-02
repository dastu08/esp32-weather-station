// APPLICATION LAYER
// Source file of the BMP180 component.

// components
#include "./al_bmp180.h"

#include "../general/general.h"
#include "../pl_i2c/pl_i2c.h"

// esp-idf
#include "esp_log.h"
// #error "include FreeRTOS.h must appear in source files
// before include task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Type of calibration parameters
typedef struct calib_param_t {
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
} calib_param_t;

// slave address of bmp180
uint8_t fd_BMP = 0x77;
// start of the eeprom
uint8_t eeprom_start = 0xAA;

// value from temperature needed for pressure conversion
int32_t b5 = 0xFFFFFFFF;

// Calibration parameter object
calib_param_t calib_param;

// Logging tag
static const char *TAG = "al_bmp180";

// function wrappers
uint8_t read_byte(uint8_t slave_addr,
                  uint8_t addr) {
    // set address on bmp180
    pl_i2c_write(slave_addr, &addr, 1);
    // read value from address
    return pl_i2c_read(slave_addr);
}

esp_err_t write_byte(uint8_t slave_addr,
                     uint8_t addr,
                     uint8_t byte) {
    // write eeprom address and content
    uint8_t bytes[2] = {addr, byte};
    return pl_i2c_write(slave_addr, bytes, 2);
}

uint16_t get_uint_param(uint8_t slave_addr,
                        uint8_t eeprom_addr) {
    // read msb and lsb
    uint16_t msb = (uint16_t)read_byte(slave_addr,
                                       eeprom_addr);
    uint16_t lsb = (uint16_t)read_byte(slave_addr,
                                       eeprom_addr + 1);
    return (msb << 8) + lsb;
}

int16_t get_int_param(uint8_t slave_addr,
                      uint8_t eeprom_addr) {
    // read msb and lsb
    int16_t msb = (int16_t)read_byte(slave_addr,
                                     eeprom_addr);
    int16_t lsb = (int16_t)read_byte(slave_addr,
                                     eeprom_addr + 1);
    return (msb << 8) + lsb;
}

void clear_calib_param() {
    calib_param.ac1 = 0;
    calib_param.ac2 = 0;
    calib_param.ac3 = 0;
    calib_param.ac4 = 0;
    calib_param.ac5 = 0;
    calib_param.ac6 = 0;
    calib_param.b1 = 0;
    calib_param.b2 = 0;
    calib_param.mb = 0;
    calib_param.mc = 0;
    calib_param.md = 0;
}

// Log the calibration parameter with verbose level.
void al_bmp180_log_calib_param() {
    ESP_LOGV(TAG, "Calibration parameter");
    ESP_LOGV(TAG, "AC1=0x%04X : %d", calib_param.ac1, calib_param.ac1);
    ESP_LOGV(TAG, "AC2=0x%04X : %d", calib_param.ac2, calib_param.ac2);
    ESP_LOGV(TAG, "AC3=0x%04X : %d", calib_param.ac3, calib_param.ac3);
    ESP_LOGV(TAG, "AC4=0x%04X : %d", calib_param.ac4, calib_param.ac4);
    ESP_LOGV(TAG, "AC5=0x%04X : %d", calib_param.ac5, calib_param.ac5);
    ESP_LOGV(TAG, "AC6=0x%04X : %d", calib_param.ac6, calib_param.ac6);
    ESP_LOGV(TAG, "B1=0x%04X : %d", calib_param.b1, calib_param.b1);
    ESP_LOGV(TAG, "B2=0x%04X : %d", calib_param.b2, calib_param.b2);
    ESP_LOGV(TAG, "MB=0x%04X : %d", calib_param.mb, calib_param.mb);
    ESP_LOGV(TAG, "MC=0x%04X : %d", calib_param.mc, calib_param.mc);
    ESP_LOGV(TAG, "MD=0x%04X : %d", calib_param.md, calib_param.md);
}

/** Read the calibration parameter from the BMP180 via I2C.

**Requirement**
    Initialize the BMP180 component with `al_bmp180_init`.

**Parameters**
    - slave_addr: 
        7bit I2C address of the bmp180 sensor
    - eeprom_start: 
        8bit eeprom address where the calibration parameters
        start

**Description**
    Reading in the calibration parameters starting from the
    start address up to 20 register further.
*/
void al_bmp180_get_calib_param(uint8_t slave_addr,
                               uint8_t eeprom_start) {
    ESP_LOGI(TAG, "Started getting calibration parameter");

    calib_param.ac1 = get_int_param(slave_addr, eeprom_start);
    calib_param.ac2 = get_int_param(slave_addr, eeprom_start + 2);
    calib_param.ac3 = get_int_param(slave_addr, eeprom_start + 4);
    calib_param.ac4 = get_uint_param(slave_addr, eeprom_start + 6);
    calib_param.ac5 = get_uint_param(slave_addr, eeprom_start + 8);
    calib_param.ac6 = get_uint_param(slave_addr, eeprom_start + 10);
    calib_param.b1 = get_int_param(slave_addr, eeprom_start + 12);
    calib_param.b2 = get_int_param(slave_addr, eeprom_start + 14);
    calib_param.mb = get_int_param(slave_addr, eeprom_start + 16);
    calib_param.mc = get_int_param(slave_addr, eeprom_start + 18);
    calib_param.md = get_int_param(slave_addr, eeprom_start + 20);

    ESP_LOGI(TAG, "Finshed getting calibration parameters");
}

esp_err_t al_bmp180_init() {
    esp_err_t err = ESP_OK;

    // set all calibration paramters to zero.
    clear_calib_param();

    // test if the communication to the BMP180 works with
    // the id 0x55 in register 0xD0
    if (0x55 != read_byte(fd_BMP, 0xD0)) {
        ESP_LOGW(TAG, "Failed init");
        err = ESP_FAIL;
    }

    // get and print calibration parameter
    al_bmp180_get_calib_param(fd_BMP, eeprom_start);
    al_bmp180_log_calib_param();

    ESP_LOGI(TAG, "Finished init");
    return err;
}

/** Get the uncompensated temperature.

**Requirement**
    Initialize the BMP180 component with `al_bmp180_init`.

**Parameters**
    - slave_addr: 7bit I2C address of the bmp180 sensor

**Return**
    - ut: uncompensated temperature

**Description**
    Write 0x2E into reg 0xF4, wait 4.5ms and then read
    registers 0xF6 (MSB), 0xF7 (LSB). Log the value to the
    console.
*/
int32_t al_bmp180_get_ut(uint8_t slave_addr) {
    int32_t ut = 0;

    // start measurement by writing value 0x2E into register
    // 0xF4
    write_byte(slave_addr, 0xF4, 0x2E);

    // delay time: 4.5ms = 4500µs
    vTaskDelay(5);

    // read out uncompensated temperature
    ut = ((int32_t)read_byte(slave_addr, 0xF6) << 8);
    ut += (int32_t)read_byte(slave_addr, 0xF7);

    ESP_LOGV(TAG, "ut=0x%08X : %d", ut, ut);

    return ut;
}

/** Get the uncompensated pressure.

**Requirement**
    Initialize the BMP180 component with `al_bmp180_init`.

**Parameters**
    - slave_addr: 
        7bit I2C address of the bmp180 sensor
    - oss: 
        oversampling setting, possible values from 0-3 see
        description

**Return**
    - up: uncompensated pressure

**Description**
    Write a different value to register 0xF4 depending on
    the value of `oss`, see the table below. Wait and then
    read 3 bytes and log the combined value.    
    
    | mode                  | oss  | internal_number_of_samples | conversion_time |
    |-----------------------|------|----------------------------|-----------------|
    | ultra_low_power       | 0    | 1                          |  4.5ms          |
    | standard              | 1    | 2                          |  7.5ms          |
    | high_resolution       | 2    | 4                          | 13.5ms          |
    | ultra_high_resolution | 3    | 8                          | 25.5ms          |
*/
int32_t al_bmp180_get_up(uint8_t slave_addr, uint8_t oss) {
    int32_t msb = 0;
    int32_t lsb = 0;
    int32_t xlsb = 0;
    int32_t up = 0;

    // start the measurement with different resolutions
    switch (oss) {
        case 0:
            write_byte(slave_addr, 0xF4, 0x34);
            // conversion time: 4.5ms = 4500µs
            vTaskDelay(5);
            break;

        case 1:
            write_byte(slave_addr, 0xF4, 0x74);
            // conversion time: 7.5ms = 7500µs
            vTaskDelay(8);
            break;

        case 2:
            write_byte(slave_addr, 0xF4, 0xB4);
            // Conversion time: 13.5ms = 13500µs
            vTaskDelay(14);
            break;

        case 3:
            write_byte(slave_addr, 0xF4, 0xF4);
            // Conversion time: 25.5ms = 25500µs
            vTaskDelay(26);
            break;

        default:
            ESP_LOGW(TAG, "the oss is incorrect.");
            break;
    }

    // read measured data
    msb = read_byte(slave_addr, 0xF6);
    lsb = read_byte(slave_addr, 0xF7);
    xlsb = read_byte(slave_addr, 0xF8);
    up = msb;
    up <<= 8;
    up += lsb;
    up <<= 8;
    up += xlsb;
    up >>= (8 - oss);
    // up = (((msb << 16) + (lsb << 8) + xlsb) >> (8 - oss));

    ESP_LOGV(TAG, "up=0x%08X : %d", up, up);

    return up;
}

int32_t al_bmp180_get_temperature() {
    int32_t ut = al_bmp180_get_ut(fd_BMP);
    int32_t t, x1, x2;
    // algorithm for the temperature
    x1 = ((ut - calib_param.ac6) * calib_param.ac5) >> 15;
    x2 = (calib_param.mc << 11) / (x1 + calib_param.md);
    b5 = x1 + x2;
    t = (b5 + 8) >> 4;

    // divide the temperature by 10 to get the value in multiples of 1.0 celsius
    ESP_LOGD(TAG,
             "temperature in degree celsius: %.1f",
             (float)t / 10);

    return t;
}

int32_t al_bmp180_get_pressure(uint8_t oss) {
    int32_t up = 0;
    int32_t p, x1, x2, x3, b3, b6;
    uint32_t b4, b7;

    if (oss > 3) {
        ESP_LOGW(TAG,
                 "Sampling mode for pressure measurement is to high: %d",
                 oss);
        //  set it to the max value
        oss = 3;
    }

    if (b5 == 0xFFFFFFFF) {
        ESP_LOGW(TAG,
                 "The value of b5 is not initialized.");
        return 0;
    }

    // get the uncompensated pressure
    up = al_bmp180_get_up(fd_BMP, oss);

    // algorithm for the pressure conversion
    b6 = b5 - 4000;
    x1 = (calib_param.b2 * ((b6 * b6) >> 12)) >> 11;
    x2 = (calib_param.ac2 * b6) >> 11;
    x3 = x1 + x2;
    b3 = ((((int32_t)calib_param.ac1 * 4 + x3) << oss) + 2) / 4;
    x1 = (calib_param.ac3 * b6) >> 13;
    x2 = (calib_param.b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (calib_param.ac4 * (uint32_t)(x3 + 32768)) >> 15;
    b7 = ((uint32_t)up - b3) * (50000 >> oss);
    if (b7 < 0x80000000) {
        p = (b7 * 2) / b4;
    } else {
        p = (b7 / b4) * 2;
    }
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    p = p + ((x1 + x2 + 3791) >> 4);

    // divide by 100 because the pressure was in Pa instead of hPa before
    ESP_LOGD(TAG,
             "Pressure in hekto pascal : %4.2f",
             (float)p / 100);

    return p;
}
