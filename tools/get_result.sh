#!/bin/sh

# 检查是否提供了正确的文件路径
if [ ! -f "$1" ]; then
  echo "Usage: sh get_result.sh [path_to_log_file]"
  exit 1
fi

LOG_FILE="$1"
found=0

# 逐行读取日志文件
while IFS= read -r line
do
  if [ $found -eq 1 ]; then
    echo "$line"
  elif echo "$line" | grep -q "run cases finish"; then
    echo "$line"
    found=1
  fi
done < "$LOG_FILE"
