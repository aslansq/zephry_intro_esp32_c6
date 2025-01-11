thisPath=$(realpath "$0")
thisDirPath=$(dirname "$thisPath")

source $thisDirPath/util.sh
if [ $? != 0 ]
then
    echo "util.sh source fail"
    echo "ERROR !!"
    exit 1
fi

if [ -z "$1" ]
then
    echo defaulting to hello world
    demoPath=$(getDefaultDemoPath)
else
    demoPath=$(realpath "$1")
fi

if [ ! -d "${demoPath}" ]
then
    ungracefulExit "demo path does not exist. ${demoPath}"
fi

cmakeFileContent=$(cat << EOF
file(GLOB demo_src "${demoPath}/*.c")
target_sources(app PRIVATE "\${demo_src}")
EOF
)

echo "$cmakeFileContent" > $(getPrjRoot)/demo.cmake
if [ $? != 0 ]
then
    ungracefulExit "demo.cmake"
fi

setupZ
if [ $? != 0 ]
then
    ungracefulExit "setupZ"
fi

west build --sysbuild -p -b esp32c6_devkitc $(getPrjRoot) --build-dir ${demoPath}/build -- \
-DOPENOCD=${OPENOCD_ESP32_PATH}/bin/openocd \
-DOPENOCD_DEFAULT_PATH=${OPENOCD_ESP32_PATH}/share/openocd/scripts
if [ $? != 0 ]
then
    ungracefulExit "build"
fi

echo SUCCESS !!