#!/bin/sh

# 获取目标目录路径
target_dir=$1

# 检查目标目录是否存在
if [ ! -d "$target_dir" ]; then
    echo "目标目录不存在: $target_dir"
    exit 1
fi

# 目标index.html文件
output_file="${target_dir}/index.html"

# 创建HTML文件的头部
echo '<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>目录列表</title>
    <style>
        body {
            font-family: Arial, sans-serif;
        }
        ul {
            list-style-type: none;
            padding: 0;
        }
        li {
            margin: 5px 0;
        }
        a {
            text-decoration: none;
            color: blue;
        }
        a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <h1>子目录列表</h1>
    <ul>' > "$output_file"

# 遍历目标目录中的子目录
for dir in "$target_dir"/*; do
    if [ -d "$dir" ]; then
        dirname=$(basename "$dir")
        if [ -f "$dir/index.html" ]; then
            echo "<li><a href=\"$dirname/index.html\">$dirname</a></li>" >> "$output_file"
        else
            echo "<li>$dirname (index.html 不存在)</li>" >> "$output_file"
        fi
    fi
done

# 完成HTML文件
echo '    </ul>
</body>
</html>' >> "$output_file"

echo "已生成 ${output_file}"

