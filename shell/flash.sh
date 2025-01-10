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

west flash --build-dir $(getPrjRoot)/build
if [ $? != 0 ]
then
    ungracefulExit "flash"
fi