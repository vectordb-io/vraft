#!/bin/bash

for file in `ls remu_*_test`; do
    echo "Running ./$file ..."
    ./"$file"
    ret=$?
    if [ ${ret} -ne 0 ]; then
        echo "exit, return ${ret}"
        exit 1
    fi

    echo "Running ./$file --enable-pre-vote ..."
    ./"$file" --enable-pre-vote
    ret=$?
    if [ ${ret} -ne 0 ]; then
        echo "exit, return ${ret}"
        exit 1
    fi
done