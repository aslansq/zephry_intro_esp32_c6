thisPath=$(realpath "$0")
thisDirPath=$(dirname "$thisPath")

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