#!/bin/bash

sh run_all_remu.sh --params=./remu_test2.param --save=yes > runlog
dirpath=`cat runlog | tail -n1`

sh /tmp/vraft_tools/get_result.sh ./runlog > ./result
sh /tmp/vraft_tools/generate_text_html.sh ./runlog > ${dirpath}/runlog.html
sh /tmp/vraft_tools/generate_text_html.sh ./result > ${dirpath}/result.html
mv ./runlog ${dirpath}
mv ./result ${dirpath}

cd /tmp/vraft_tools/
sh generate_casedir.sh ${dirpath}
cd -

mv ${dirpath} /usr/local/nginx/html
cd /tmp/vraft_tools/
sh generate_casedir.sh /usr/local/nginx/html
cd -

echo "ok"



