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

note=""

for demo in ${demos}
do
    if [ ! -d "${demo}" ]
    then
        ungracefulExit "demo path does not exist. ${demo}"
    fi
    echo "building ${demo}"
    echo "========================================"
    ${thisDirPath}/build.sh ${demo}
    if [ $? != 0 ]
    then
        ungracefulExit "${demo} build"
    else
        note="$note\nbuild ${demo} success"
        echo "build ${demo} success"
        echo "========================================"
    fi
done

echo -e "$note\n"
echo "All demos rebuilt successfully!"