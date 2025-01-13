thisPath=$(realpath "$0")
thisDirPath=$(dirname "$thisPath")

source $thisDirPath/util.sh
if [ $? != 0 ]
then
    echo "util.sh source fail"
    echo "ERROR !!"
    exit 1
fi

# always execute from project root
cd $(getPrjRoot)
if [ $? != 0 ]
then
    ungracefulExit "cd project root"
fi

myzephyrproject=$(getPrjRoot)/..

$OPENOCD_ESP32_PATH/bin/openocd \
-s $myzephyrproject/zephyr/boards/espressif/esp32c6_devkitc/support \
-s $OPENOCD_ESP32_PATH/share/openocd/scripts \
-f $myzephyrproject/zephyr/boards/espressif/esp32c6_devkitc/support/openocd.cfg \
-c 'tcl port 6333' \
-c 'telnet_port 4444' \
-c 'gdb port 3333'