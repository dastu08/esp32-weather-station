// PROTOCOL LAYER
// Header file of the I2C component.

#ifndef _PL_I2C_H_
#define _PL_I2C_H_

#include "driver/gpio.h"

/** Initialize the I2C driver.


**Parameters**
    - sda_pin: pin number of SDA
    - scl_pin: pin number of SCL
    - clock_speed: frequency of clock

**Description**
    Creates an `i2c_config_t` object in master mode. Then
    apply the configurataion and install the driver.
*/
void pl_i2c_init(gpio_num_t sda_pin,
                 gpio_num_t scl_pin,
                 uint32_t clock_speed);

/** Write data to the i2c slave.

**Requirements**
    The I2C driver needs to be `initialized with
    pl_i2c_init`.

**Parameters**
    - slave_addr: 7bit address of the slave
    - bytes: data to write
    - length: length of `bytes`


**Return**
    - err: 
        the `esp_err_t` of the executing
        `i2c_master_cmd_begin`.


**Description**
    Create an `i2c_cmd_handle_t` command link. Generate
    start bit, write message, generate stop bit. Delete the
    command link. Log the send message.
*/
esp_err_t pl_i2c_write(uint8_t slave_addr,
                       uint8_t *bytes,
                       int length);

/** Read one byte of data from the i2c slave.

**Requirements**
    The I2C driver needs to be `initialized with
    pl_i2c_init`.

**Parameters**
    - slave_addr: 7bit address of the slave

**Return**
    - byte: the value of the read byte

**Description**
    Create an `i2c_cmd_handle_t` command link. Generate
    start bit, read message, generate stop bit. Delete the
    command link. Log the read message.
*/
uint8_t pl_i2c_read(uint8_t slave_addr);

#endif  // _PL_I2C_H_