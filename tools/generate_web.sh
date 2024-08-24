#!/bin/bash

reconfig="0"
for arg in "$@"
do
  if [ "$arg" = "--reconfig" ]; then
    reconfig="1"
  fi
done

dir="/tmp/remu_test_dir/"

rm -rf "${dir}/remu_web"
mkdir -p "${dir}/remu_web"

cp ./*.css "${dir}/remu_web"

cp "${dir}/log/remu.log" "${dir}/remu_web"
cp "${dir}/log/remu.log.global" "${dir}/remu_web"
#cp "${dir}/log/remu.log.sm" "${dir}/remu_web"
#cp ${dir}/log/keys.* "${dir}/remu_web" 2>/dev/null
cp ${dir}/log/*.sm "${dir}/remu_web" 2>/dev/null
cp ${dir}/log/*.message "${dir}/remu_web" 2>/dev/null
cp ${dir}/log/*.planttext "${dir}/remu_web" 2>/dev/null

if [ "$reconfig" = "1" ]; then
    node generate_global_cc.js "${dir}/remu_web/remu.log.global" "${dir}/remu_web/global.html.body" 
else
    node generate_global.js "${dir}/remu_web/remu.log.global" "${dir}/remu_web/global.html.body" 
fi

cat ./html/global.html.head "${dir}/remu_web/global.html.body" ./html/global.html.tail > "${dir}/remu_web/global.html"
rm "${dir}/remu_web/global.html.body"

for sm in `ls ${dir}/log/*.sm`;do
    echo "processing ${sm} ..."
    node generate_node.js "${sm}" "${dir}/remu_web/node.html.body" 
    
    if [ "$reconfig" = "1" ]; then
        node generate_node2_cc.js "${sm}" "${dir}/remu_web/node.html.body2"
    else
        node generate_node2.js "${sm}" "${dir}/remu_web/node.html.body2"
    fi

    cat ./html/node.html.head "${dir}/remu_web/node.html.body" "${dir}/remu_web/node.html.body2" ./html/node.html.tail > ${sm}.html
    mv ${sm}.html "${dir}/remu_web"
    rm "${dir}/remu_web/node.html.body" "${dir}/remu_web/node.html.body2"
done

node generate_message_flow.js ${dir}/log/remu.log.sm.message > ${dir}/remu_web/message_flow_bk.html

plantuml -tsvg ${dir}/log/remu.log.sm.puml
cp ${dir}/log/remu.log.sm.svg ${dir}/remu_web/message_flow.svg