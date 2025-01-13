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

# always execute from project root
cd $(getPrjRoot)
if [ $? != 0 ]
then
    ungracefulExit "cd project root"
fi

west espressif monitor