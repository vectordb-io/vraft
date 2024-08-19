#!/bin/bash

dir="/tmp/remu_test_dir/"
sh update_file_name.sh "${dir}/remu_web"
node remu_server.js "${dir}/remu_web"
