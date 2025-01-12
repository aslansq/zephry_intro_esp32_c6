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

$thisDirPath/build.sh ${demoPath} && $thisDirPath/flash.sh && $thisDirPath/mon.sh