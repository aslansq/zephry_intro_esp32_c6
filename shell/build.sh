thisPath=$(realpath "$0")
thisDirPath=$(dirname "$thisPath")

source $thisDirPath/util.sh
if [ $? != 0 ]
then
    echo "util.sh source fail"
    echo "ERROR !!"
    exit 1
fi

setupZ
if [ $? != 0 ]
then
    ungracefulExit "setupZ"
fi

west build --sysbuild -p -b esp32c6_devkitc $(getPrjRoot) -- \
-DOPENOCD=/home/aslan/tool/openocd-esp32-linux-amd64-0.12.0-esp32-20241016/openocd-esp32/bin/openocd \
-DOPENOCD_DEFAULT_PATH=/home/aslan/tool/openocd-esp32-linux-amd64-0.12.0-esp32-20241016/openocd-esp32/share/openocd/scripts
if [ $? != 0 ]
then
    ungracefulExit "build"
fi

echo SUCCESS !!