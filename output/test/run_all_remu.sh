#!/bin/bash

FILE_PATH="./remu_test.param"
SAVE="no"

for arg in "$@"
do
  # 检查参数格式是否为 --param=value
  if echo "$arg" | grep -q -- "--.*=.*"; then
    # 使用 = 作为分隔符提取param和value
    param=$(echo "$arg" | cut -d'=' -f1 | sed 's/^--//')
    value=$(echo "$arg" | cut -d'=' -f2)
    echo "$param : $value"
    if [ "$param" = "params" ]; then
        FILE_PATH="$value"
    fi

    if [ "$param" = "save" ]; then
        SAVE="$value"
    fi
  else
    echo "invalid parameter: $arg"
    echo "Usage:"
    echo "sh $0 --params=./remu_test.param --save=yes"
    exit 1
  fi
done

if [ ! -f "$FILE_PATH" ]; then
    echo "file $FILE_PATH not exist"
    exit 1
fi

current_time=$(date +"%Y-%m-%d-%H-%M-%S")
case_dir=~/remu_cases.${current_time}
mkdir -p ${case_dir}

all_case=""

# case count
count=0

# start_time
start_time=$(date +%s)

save_case_data() {
    if [ "$SAVE" = "yes" ]; then
        cpdir=$(echo "${cmd}" | tr -d ' ')
        cd /tmp/vraft_tools/
        if echo "$cpdir" | grep -q "reconfig"; then
            echo "----------- reconfig case"
            sh one_key.sh --reconfig --noweb
        else
            sh one_key.sh --noweb
        fi
        cd -

        echo "------move case data to ${case_dir}"
        mkdir -p ${case_dir}/${cpdir}
        mv /tmp/remu_test_dir/remu_web/* ${case_dir}/${cpdir}
    fi
}

for file in `ls remu_*_test`; do
    while IFS= read -r param || [ -n "$param" ]; do
        #echo "$param"
        cmd="./${file} ${param}"
        echo "---------------------------->>>>> Running ${cmd} ..."
        count=$((count + 1))
        ${cmd}
        ret=$?
        if [ ${ret} -ne 0 ]; then
            echo ""
            echo "----------------------------xxxxxxxxxxxxx ${cmd} failed, return ${ret}"

            echo "already run case:"
            echo ${all_case}
            echo ""

            # end_time
            end_time=$(date +%s)

            time_diff=$((end_time - start_time))
            echo "run ${count} case, elapsed time:${time_diff} seconds"

            save_case_data

            exit 1
        else
            all_case="${all_case}\n${cmd}"
            echo ""
            echo "----------------------------+++++++++++++++ ${cmd} ok, return ${ret}"

            save_case_data
        fi
    done < "$FILE_PATH"
done

echo "run case:"
echo ${all_case}
echo ""

# end_time
end_time=$(date +%s)

# time_diff
time_diff=$((end_time - start_time))
echo "run ${count} case, elapsed time:${time_diff} seconds"

if [ "$SAVE" = "yes" ]; then
    echo "------move case data to ${case_dir}"
fi

echo ""
echo "all case ok, perfect!!!"