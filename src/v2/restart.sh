#!/bin/bash

mkdir -p /tmp/vraft
rm -rf /tmp/vraft/*

killall vraft_server

nohup ./vraft_server --me=127.0.0.1:38000 --peers=127.0.0.1:38001,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38000 1>38000.msg 2>38000.err &
nohup ./vraft_server --me=127.0.0.1:38001 --peers=127.0.0.1:38000,127.0.0.1:38002 --path=/tmp/vraft/127.0.0.1:38001 1>38001.msg 2>38001.err &
nohup ./vraft_server --me=127.0.0.1:38002 --peers=127.0.0.1:38000,127.0.0.1:38001 --path=/tmp/vraft/127.0.0.1:38002 1>38002.msg 2>38002.err &


