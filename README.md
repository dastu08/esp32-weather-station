# ESP32 DevKitC
This is an example project with the ESP32 chip. Try out some
features of it and get experience.
* [Framework](#framework)
* [Hardware](#hardware)
* [VS Code Extension](#vs-code-extension)

------------------------------

## Framework
The framework used in this project is the ESP-IDF by espressif that can be found on GitHub and has an extensive documentation  
=> https://github.com/espressif/esp-idf  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/get-started/index.html

This project uses version 4.2.
```bash
git clone --recurisve --branch v4.2 https://github.com/espressif/esp-idf.git
```

The following header files are part of `esp_common` component and **do not need** to be added to required lists in `CMakeLists.txt` of components.
* `esp_err.h`

### Toolchain
The setup needs the name `python` so set the alternatives and then install the toolchain for the local user inside the `esp/esp-idf` directory. 
```bash
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10 && alias pip=pip3
./install.sh
```
After the installation you need to source `esp/esp-idf/export.sh`. Create an alias in `.bashrc`.
```
alias get_idf='. $HOME/esp/esp-idf/export.sh'
```

> For the gdb debugger to work you have to install the
> package `libpython2.7-dev`.

### Project Configuration
You can configure the framework for the project in an interactive terminal. Then `build`, `flash` and `monitor` the project. The `port` for flashing is `/dev/ttyUSB0` on Ubuntu/openSUSE.  
```
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py flash -p <port>
idf.py monitor -p <port>
idf.py flash monitor -p <port>
idf.py size-components
```
Further documentation for monitor.  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/tools/idf-monitor.html

### WiFi
The wifi bundle uses the **LwIP stack** with `esp_netif` and **BSD Sockets** for UDP/TCP communication. Time syncronization is done via `sntp`.  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/wifi.html  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-reference/network/esp_netif.html  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/lwip.html  
=> https://pubs.opengroup.org/onlinepubs/007908799/xns/socket.html  
=> https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html?highlight=system%20time

------------------------------------------

## Hardware
### ESP32-DevKitC V4
Get started with the _ESP32-DevKitC V4_ board with has the _ESP-WROOM-32_ chip. The I2C Bus works on `3.3 V`.  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/hw-reference/modules-and-boards.html#esp32-devkitc-v4  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/hw-reference/modules-and-boards.html#esp-modules-and-boards-esp32-wroom-32

### ESP-Prog
For debugging use the _ESP-Prog_ board which has JTAG debugging capabilities. The debugging chip uses two ports: `/dev/ttyUSB0` is for the JTAG interface and
`/dev/ttyUSB1` for the flashing. To connect the correct pins see the third link.  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/jtag-debugging/index.html  
=> https://github.com/espressif/esp-dev-kits/blob/master/esp-prog/docs/ESP-Prog_guide_en.md  
=> https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-guides/jtag-debugging/configure-other-jtag.html  
=> https://docs.platformio.org/en/latest/faq.html#faq-udev-rules

Make sure to apply the `udev` rule to avoid permission issues and then start the openocd service. Note this has to run when starting the `gdb`. 
```
openocd -f interface/ftdi/esp32_devkitj_v1.cfg -f target/esp32.cfg
```
You can also flash the ESP32 via JTAG.
```
openocd -f interface/ftdi/esp32_devkitj_v1.cfg -f target/esp32.cfg -c "program_esp build/esp32-devkitc.bin 0x10000 verify exit"
```
Start the debugger
```
xtensa-esp32-elf-gdb -x gdbinit build/esp32-devkitc.elf
```

| ESP-PROG | DevKit C |
|----------|----------|
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


=> https://github.com/espressif/esp-dev-kits/blob/master/esp-prog/docs/_static/program_pin.png program pins  
=> https://github.com/espressif/esp-dev-kits/blob/master/esp-prog/docs/_static/JTAG_pin.png JTAG pins


### BMP180 Sensor
Temperature and air pressure sensor with an I2C interface.
* I2C slave address: `0x77`
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
