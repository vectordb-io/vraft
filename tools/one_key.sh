#!/ bin / bash

reconfig="0"
noweb="0"

for arg in "$@"
do
    if [ "$arg" = "--reconfig" ]; then
        reconfig="1"
    fi

    if [ "$arg" = "--noweb" ]; then
        noweb="1"
    fi
done

sh analyze.sh

if [ "$reconfig" = "1" ]; then
    sh generate_web.sh --reconfig
else
    sh generate_web.sh
fi

if [ "$noweb" = "0" ]; then
    sh start_remu_web.sh
fi

