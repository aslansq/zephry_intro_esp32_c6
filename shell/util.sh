thisPath=$(realpath "$0")
thisDirPath=$(dirname "$thisPath")

demos="\
00_hello_world \
01_blink \
02_rgb_led \
03_task_scheduling \
03_task_scheduling_challenge \
04_memory_1 \
04_memory_2 \
04_memory_3 \
04_memory_4 \
04_memory_challenge"

echoerr() { echo "$@" 1>&2; }

ungracefulExit()
{
    echoerr "Failure $@"
    echoerr ERROR!!
    exit 1
}

echoWTab()
{
    while IFS= read -r line; do
    echo -e "\t$line"
    done <<< "$@"
}

getDefaultDemoPath()
{
    echo $thisDirPath/../00_hello_world
}

getPrjRoot()
{
    echo $thisDirPath/../
}

setupZ()
{
    zRoot=$thisDirPath/../../
    if [ ! -d $zRoot/zephyr/boards/espressif/esp32c6_devkitc ]
    then
        ungracefulExit "esp32c6_devkitc not found"
    fi

    source $zRoot/.venv/bin/activate
    if [ $? != 0 ]
    then
        ungracefulExit "py virtual env"
    fi
}