# zephyr_intro_esp32c6
Zephyr RTOS introduction

This repository implements concepts from Digi-Key RTOS introduction in Zephyr RTOS.

[Introduction to RTOS | Digi-Key Electronics](https://www.youtube.com/playlist?list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz)

## Prerequisites
1. Follow [getting](https://docs.zephyrproject.org/latest/develop/getting_started/index.html) started guide from zephyrproject to setup environment.
2. Clone project under zephyr project.  
```
cd ~/zephyrproject && git clone https://github.com/aslansq/zephyr_intro_esp32c6.git
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
cd ~/zephyrproject/zephyr_intro_esp32c6 && chmod +x shell/*.sh
```

## How To Run
```
$ ./shell/build.sh ./03_task_scheduling && ./shell/flash.sh && ./shell/mon.sh
*Barka*deer *brig A*rr bo*oty r*um.
Barkadeer brig Arr booty rum.
*****B*arkade*er br*ig Ar*r boot*y rum*.
******Barkadeer brig Arr booty rum.
Barkadeer brig Arr booty rum.
*********Bar*kadeer* brig* Arr *booty *rum.
***Barkadeer brig Arr booty rum.
**Ba*rkade*er br*ig Arr* boot*y rum*.
********B*arkad*eer br*ig Ar*r boo*ty rum*.
thread1 abort
****************************
```