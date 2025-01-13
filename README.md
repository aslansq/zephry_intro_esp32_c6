# zephyr_intro_esp32c6
Zephyr RTOS introduction

This repository implements concepts from Digi-Key RTOS introduction in Zephyr RTOS.

See FreeRTOS version of this at [freertos_intro_qemu](https://github.com/aslansq/freertos_intro_qemu)

[Introduction to RTOS | Digi-Key Electronics](https://www.youtube.com/playlist?list=PLEBQazB0HUyQ4hAPU1cJED6t3DU0h34bz)

## Demos
**Some of the demos make board application to fail. Hold user button, then press reset button to switch download mode to recover.**
|Name|Description|
|-|-|
|00_hello_world|Prints hello world every 1 second|
|01_blink|Toggles gpio10 every 1 second|
|02_rgb_led|Changes color of the led every 1 second|
|03_task_scheduling|Demos pre-emption of threads|
|03_task_scheduling_challenge|Reads delay value from console and changes colors with that delay|
|04_memory_1|Stack overflow by infinite recursive call|
|04_memory_2|Stack overflow by allocating huge arrary in a thread|
|04_memory_3|Heap overflow uint32 at a time in a thread|
|04_memory_4|Heap overflow single shot in a thread|
|04_memory_challenge|Sharing message between threads|
|05_pipe|Pipe is equivalent of FreeRTOS xQueue. Communication between threads using pipes|
|05_pipe_challenge|Pipe is equivalent of FreeRTOS xQueue. One thread controls led, other thread handles user input. Two threads communicate using pipes.|
|06_mutex_race|Race conditions demo. Shared variable increased and printed to console.|
|06_mutex|Mutex used to solve race condition at 06_mutex_race|
|06_mutex_challenge|Creating a blink thread inside of setup thread. Passing an argument to blink thread from setup's stack. Mutex used to prevent delay variable does not go out of scope before blink thread has read.|
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

[comment]: <> (ESPRESSIF)
[comment]: <> (ESP32-C6-DevKitC-1)