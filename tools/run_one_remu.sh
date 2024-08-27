#!/bin/bash

# start_time
start_time=$(date +%s)
count=0

cmd="$1"
${cmd}
ret=$?
if [ ${ret} -ne 0 ]; then
    echo "run ${cmd} error"    
else
    echo "run ${cmd} ok"
    count=$((count + 1))
fi

cmd="$1 --enable-pre-vote"
${cmd}
ret=$?
if [ ${ret} -ne 0 ]; then
    echo "run ${cmd} error"    
else
    echo "run ${cmd} ok"
    count=$((count + 1))
fi

cmd="$1 --enable-interval-check"
${cmd}
ret=$?
if [ ${ret} -ne 0 ]; then
    echo "run ${cmd} error"    
else
    echo "run ${cmd} ok"
    count=$((count + 1))
fi

cmd="$1 --enable-pre-vote --enable-interval-check"
${cmd}
ret=$?
if [ ${ret} -ne 0 ]; then
    echo "run ${cmd} error"    
else
    echo "run ${cmd} ok"
    count=$((count + 1))
fi

cmd="$1 --node-num=5"
${cmd}
ret=$?
if [ ${ret} -ne 0 ]; then
    echo "run ${cmd} error"    
else
    echo "run ${cmd} ok"
    count=$((count + 1))
fi

cmd="$1 --enable-pre-vote --node-num=5"
${cmd}
ret=$?
if [ ${ret} -ne 0 ]; then
    echo "run ${cmd} error"    
else
    echo "run ${cmd} ok"
    count=$((count + 1))
fi

cmd="$1 --enable-interval-check --node-num=5"
${cmd}
ret=$?
if [ ${ret} -ne 0 ]; then
    echo "run ${cmd} error"    
else
    echo "run ${cmd} ok"
    count=$((count + 1))
fi

cmd="$1 --enable-pre-vote --enable-interval-check --node-num=5"
${cmd}
ret=$?
if [ ${ret} -ne 0 ]; then
    echo "run ${cmd} error"    
else
    echo "run ${cmd} ok"
    count=$((count + 1))
fi

# end_time
end_time=$(date +%s)

# time_diff
time_diff=$((end_time - start_time))
echo "elapsed time:${time_diff} seconds"

echo ""
echo "run ${count} case ok!"
