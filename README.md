# zephry_intro_esp32c6
Zephry RTOS introduction

## Prerequisites
1. Follow [getting](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) started guide from zephryproject to setup environment.
2. Clone project under zephry project.  
```
cd ~/zephyrproject & git clone https://github.com/aslansq/zephry_intro_esp32c6.git
```
3. Define enviroment variable OPENOCD_ESP32_PATH.
```
export OPENOCD_ESP32_PATH=/home/aslan/tool/openocd-esp32-linux-amd64-0.12.0-esp32-20241016/openocd-esp32
```
```
$  tree -L 2 $OPENOCD_ESP32_PATH
/home/aslan/tool/openocd-esp32-linux-amd64-0.12.0-esp32-20241016/openocd-esp32
├── bin
│   └── openocd
└── share
    ├── info
    ├── man
    └── openocd
```
4. Give execute permission files under shell.
```
cd ~/zephyrproject/zephry_intro_esp32c6 & chmod +x shell/*.sh
```