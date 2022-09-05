# ESP32 Weather Station
This is a project with the ESP32 chip onboard the **ESP32-DevKitC** and a 
temperature and pressure sensor **BMP180**. It uses the ESP-IDF framework.
- [ESP32 Weather Station](#esp32-weather-station)
  - [Usage](#usage)
  - [Framework](#framework)
    - [Toolchain Troubleshooting (Ubuntu)](#toolchain-troubleshooting-ubuntu)
    - [Project Configuration](#project-configuration)
    - [WiFi](#wifi)
  - [Hardware](#hardware)
    - [ESP32-DevKitC V4](#esp32-devkitc-v4)
    - [ESP-Prog](#esp-prog)
      - [Pin connection for debugging](#pin-connection-for-debugging)
    - [BMP180 Sensor](#bmp180-sensor)
  - [VS Code Extension](#vs-code-extension)

------------------------------
## Usage

1. Connect the `BMP180` to the `ESP32-DevKitC`. The default pins for **I2C** 
    are `GPIO 19` for **SDA** and `GPIO 18` for **SCL**. This is set in 
    `main.c` by calling
    ```c
    pl_i2c_init(GPIO_NUM_19, GPIO_NUM_18, 100000);
    ```
2. The ESP32 needs to connect to an existing WiFi network. See the section 
    [Project Configuration](#project-configuration) for the steps to enter 
    the _ssid_ and the _password_.
3. Communicate with the ESP32 via **UDP**. Both sending and receiving are 
    done over the same port. The port is set in `main.c`. 
    ```c
    #define UDP_PORT 50000
    ```
4. The **UDP** communication consists of **JSON** objects that are send as 
    strings. The following listing shows all the current options. 
    _The numerical values for the interval variables are in seconds._
    ```json
    {"type":"get", "quantity":"temperature"}
    {"type":"get", "quantity":"pressure"}
    {"type":"get", "quantity":["temperature", "pressure"]}
    {"type":"set", "name":"heartbeat", "value":"on"}
    {"type":"set", "name":"heartbeat", "value":"off"}
    {"type":"set", "name":"heartbeat_interval", "value": 30}
    {"type":"set", "name":"measurement_interval", "value": 5}
    ```
5. The return objects have the following syntax.   
    Response to a `get` request which has only a single element in the list 
    of `quantity`.
    ```json
    {
        "type":"response",
        "time":"2021-05-04 20:11:20 CET",
        "quantity": [{
            "name":"temperature",
            "value": 21.5,
            "unit": "celsius"
        }]
    }
    ```
    Respone from a periodic `measurement` which holds a list of measured 
    quantities in `quantity`.
    ```json
    {
        "type":"measurement",
        "time":"2021-10-18 14:41:29 CET",
        "quantity":[
            {
                "name":"temperature",
                "value": 21.6,
                "unit":"celsius"
            },
            {
                "name":"pressure",
                "value": 1019.310,
                "unit":"hPa"
            }]
    }
    ```
    For debug purposes the ESP32 also sends objects with type `heartbeat` 
    which do not contain any more information than the `type` and the `time`.
    ```json
    {
        "type":"heartbeat",
        "time":"2021-10-18 14:42:53 CET"
    }
    ```
6. The UDP traffic is encrypted `AES-256-CBC` mode. The 32 byte key is set 
   with `menuconfig`. The buffer length `BUFFER_LENGTH` in `al_crypto`
   determines the length of the outgoing message. The incoming messages
   can be as large as the `BUFFER_LENGTH + 16` for they contain the initialization vector as the first 16 bytes.

-------------------------

## Framework
The framework used in this project is the **ESP-IDF** by espressif that can be
found on GitHub and has an extensive documentation.  
=> https://github.com/espressif/esp-idf  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/get-started/index.html

This project uses **version 4.2** of the ESP-IDF.
```bash
git clone --recurisve --branch v4.2 https://github.com/espressif/esp-idf.git
```

### Toolchain Troubleshooting (Ubuntu)
> Make sure to have the packages `python3-virtualenv` installed if you have an 
> error `No module pip`.

The setup needs the name `python` so set the alternatives.
```bash
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10 && alias pip=pip3
```
Then install the toolchain for the local user inside the `esp/esp-idf` 
directory by executing 
```bash
./install.sh
```
After the installation you need to source `esp/esp-idf/export.sh`. Create an 
alias in `.bashrc`.
```bash
alias get-idf='. $HOME/esp/esp-idf/export.sh'
```
> For the gdb debugger to work you have to install the package `libpython2.7-dev`.


### Project Configuration
You can configure the framework for the project in an interactive terminal. 
Then `build`, `flash` and `monitor` the project. The `port` for flashing is 
`/dev/ttyUSB0` on Ubuntu.  
```bash 
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py flash -p <port>
idf.py monitor -p <port>
idf.py flash monitor -p <port>
idf.py size-components
```
> The WiFi configuration of the `menuconfig` is located under   
> `Component config ---> Wifi Connect Config --->  WiFi SSID /WiFi Password`.   
> The fast scan method should work fine.

Further documentation for `monitor`.  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/tools/idf-monitor.html

> **Notice when writing components:**  
> The following header files are part of `esp_common` component and 
> **do not need** to be added to required lists in `CMakeLists.txt` of 
> components.
> - `esp_err.h`



### WiFi
The WiFi bundle uses the **LwIP stack** with `esp_netif` and **BSD Sockets** 
for UDP/TCP communication. Time syncronization is done via `sntp`.  

=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/wifi.html  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-reference/network/esp_netif.html  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/lwip.html  
=> https://pubs.opengroup.org/onlinepubs/007908799/xns/socket.html  
=> https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html?highlight=system%20time

------------------------------------------

## Hardware
### ESP32-DevKitC V4
Get started with the **ESP32-DevKitC V4** board with has the _ESP-WROOM-32_ 
chip. The _I2C_ Bus works on `3.3 V`.  

=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/hw-reference/modules-and-boards.html#esp32-devkitc-v4  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/hw-reference/modules-and-boards.html#esp-modules-and-boards-esp32-wroom-32

### ESP-Prog
For debugging use the **ESP-Prog** board which has _JTAG_ debugging 
capabilities. The debugging chip uses two ports:   
- `/dev/ttyUSB0` is for the JTAG interface and
- `/dev/ttyUSB1` for the flashing.   

To connect the correct pins see the third link.

=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/jtag-debugging/index.html  
=> https://github.com/espressif/esp-dev-kits/blob/master/esp-prog/docs/ESP-Prog_guide_en.md  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/jtag-debugging/configure-other-jtag.html  
=> https://docs.platformio.org/en/latest/faq.html#faq-udev-rules

Make sure to apply the `udev` rule to avoid permission issues and then start 
the `openocd` service. Note this has to run when starting the `gdb`. 
```bash
openocd -f interface/ftdi/esp32_devkitj_v1.cfg -f target/esp32.cfg
```
You can also flash the ESP32 via JTAG.
```bash
openocd -f interface/ftdi/esp32_devkitj_v1.cfg -f target/esp32.cfg -c "program_esp build/esp32-devkitc.bin 0x10000 verify exit"
```
Start the debugger
```bash
xtensa-esp32-elf-gdb -x gdbinit build/esp32-devkitc.elf
```

#### Pin connection for debugging
| ESP-PROG | DevKit C |
| -------- | -------- |
| TDO      | G15      |
| TDI      | G12      |
| TCK      | G13      |
| TMS      | G14      |
| EN       | EN       |
| TXD      | TXD      |
| RXD      | RXD      |
| VDD      | 3V3      |
| GND      | GND      |
| IO0      | G0       |

=> https://github.com/espressif/esp-dev-kits/blob/master/esp-prog/docs/_static/program_pin.png   
=> https://github.com/espressif/esp-dev-kits/blob/master/esp-prog/docs/_static/JTAG_pin.png 


### BMP180 Sensor
Temperature and air pressure sensor with an I2C interface.
* I2C address: `0x77`
* I2C clock speed max: `3.4 MHz`

-----------------------------

## VS Code Extension
See GitHub on debugging with the extension.  

=> https://github.com/espressif/vscode-esp-idf-extension  
=> https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/DEBUGGING.md

Useful `settings.json`
```json
{
    "idf.port": "/dev/ttyUSB1",
    "idf.adapterTargetName": "esp32",
    "idf.flashType": "UART",
    "idf.openOcdConfigs": [
        "interface/ftdi/esp32_devkitj_v1.cfg",
        "target/esp32.cfg"
    ]
}
```

Useful `launch.json`
```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "GDB",
            "type": "cppdbg",
            "request": "launch",
            "MIMode": "gdb",
            "miDebuggerPath": "${command:espIdf.getXtensaGdb}",
            "program": "${workspaceFolder}/build/${command:espIdf.getProjectName}.elf",
            "cwd": "${workspaceFolder}",
            "environment": [
                {
                    "name": "PATH",
                    "value": "${config:idf.customExtraPaths}"
                }
            ],
            "setupCommands": [
                { "text": "target remote :3333" },
                { "text": "set remote hardware-watchpoint-limit 2"},
                { "text": "mon reset halt" },
                { "text": "thb app_main" },
                { "text": "flushregs" }
            ],
            "externalConsole": false,
            "logging": {
                "exceptions": true,
                "moduleLoad": true,
                "programOutput": true,
                "engineLogging": false,
                "trace": false,
                "traceResponse": false
            },
        }
    ]
}
```
