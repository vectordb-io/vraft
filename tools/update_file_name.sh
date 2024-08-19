#!/bin/bash

# 检查是否提供了参数
if [ -z "$1" ]; then
    echo "请提供一个目录路径。用法: sh a.sh /tmp/path"
    exit 1
fi

# 获取目录路径
TARGET_DIR="$1"

# 检查目录是否存在
if [ ! -d "$TARGET_DIR" ]; then
    echo "目录不存在: $TARGET_DIR"
    exit 1
fi

# 遍历目标目录中的所有文件
for file in "$TARGET_DIR"/*; do
    # 检查文件名是否包含 '#' 或 ':'
    if echo "$file" | grep -q "[#:]"; then
        # 提取文件名部分
        filename=$(basename "$file")
        # 使用 `sed` 替换文件名中的 '#' 和 ':' 为 '_'
        new_name=$(echo "$filename" | sed 's/[#:]/_/g')
        # 打印旧文件名和新文件名
        echo "Renaming '$filename' to '$new_name'"
        # 重命名文件
        mv "$file" "$TARGET_DIR/$new_name"
    fi
done
