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
    ungracefulExit "demo not given"
else
    demoPath=$(realpath "$1")
fi

if [ ! -d "${demoPath}" ]
then
    ungracefulExit "demo path does not exist. ${demoPath}"
fi

setupZ
if [ $? != 0 ]
then
    ungracefulExit "setupZ"
fi

west flash --build-dir ${demoPath}/build
if [ $? != 0 ]
then
    ungracefulExit "flash"
fi