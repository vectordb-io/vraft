#!/bin/bash

for file in `ls remu_*_test`; do
    cmd="./${file}"
    echo ""
    echo "++++++++++++------------>>>>> Running ${cmd} ..."

    ${cmd}
    ret=$?
    if [ ${ret} -ne 0 ]; then
        echo ""
        echo "++++++++++++------------>>>>> ${cmd} exit, return ${ret}"
        exit 1
    else
        echo ""
        echo "++++++++++++------------>>>>> ${cmd} ok, return ${ret}"
    fi

    cmd="./${file} --enable-pre-vote"
    echo ""
    echo "++++++++++++------------>>>>> Running ${cmd} ..."

    ${cmd}
    ret=$?
    if [ ${ret} -ne 0 ]; then
        echo ""
        echo "++++++++++++------------>>>>> ${cmd} exit, return ${ret}"
        exit 1
    else
        echo ""
        echo "++++++++++++------------>>>>> ${cmd} ok, return ${ret}"
    fi
done