#!/bin/bash

sh run_all_remu.sh --params=./remu_test.param --save=yes > execute_log
dirpath=`cat execute_log | tail -n1`

sh /tmp/vraft_tools/get_result.sh ./execute_log > ./execute_result
sh /tmp/vraft_tools/generate_text_html.sh ./execute_log > ${dirpath}/execute_log.html
sh /tmp/vraft_tools/generate_text_html.sh ./execute_result > ${dirpath}/execute_result.html
mv ./execute_log ${dirpath}
mv ./execute_result ${dirpath}

cd /tmp/vraft_tools/
sh generate_casedir.sh ${dirpath}
cd -

mv ${dirpath} /usr/local/nginx/html
cd /tmp/vraft_tools/
sh generate_casedir.sh /usr/local/nginx/html
cd -

echo "ok"



