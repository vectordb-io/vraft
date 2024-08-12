#!/bin/bash

all_case=""

# case count
count=0

# start_time
start_time=$(date +%s)

for file in `ls remu_*_test`; do
    cmd="./${file}"
    echo ""
    echo "++++++++++++------------>>>>> Running ${cmd} ..."

    count=$((count + 1))
    ${cmd}
    ret=$?
    if [ ${ret} -ne 0 ]; then
        echo ""
        echo "++++++++++++------------>>>>> ${cmd} exit, return ${ret}"

        echo "run case:"
        echo ${all_case}
        echo ""

        # end_time
        end_time=$(date +%s)

        time_diff=$((end_time - start_time))
        echo "run ${count} case, elapsed time:${time_diff} seconds"
        exit 1
    else
        all_case="${all_case}\n${cmd}"
        echo ""
        echo "++++++++++++------------>>>>> ${cmd} ok, return ${ret}"
    fi

    cmd="./${file} --enable-pre-vote"
    echo ""
    echo "++++++++++++------------>>>>> Running ${cmd} ..."

    count=$((count + 1))
    ${cmd}
    ret=$?
    if [ ${ret} -ne 0 ]; then
        echo ""
        echo "++++++++++++------------>>>>> ${cmd} exit, return ${ret}"

        echo "run case:"
        echo ${all_case}
        echo ""

        # end_time
        end_time=$(date +%s)

        time_diff=$((end_time - start_time))
        echo "run ${count} case, elapsed time:${time_diff} seconds"
        exit 1
    else
        all_case="${all_case}\n${cmd}"
        echo ""
        echo "++++++++++++------------>>>>> ${cmd} ok, return ${ret}"
    fi
done

echo "run case:"
echo ${all_case}
echo ""

# end_time
end_time=$(date +%s)

# time_diff
time_diff=$((end_time - start_time))
echo "run ${count} case, elapsed time:${time_diff} seconds"

echo ""
echo "all case ok, perfect!!!"
