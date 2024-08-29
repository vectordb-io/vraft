#!/bin/sh

# 检查是否传入了目标目录参数
if [ $# -eq 0 ]; then
  echo "Usage: $0 target_directory"
  exit 1
fi

# 从命令行参数获取目标目录
target_dir="$1"
output_file="${target_dir}/index.html"

# 创建HTML文件的头部
echo '<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>文件列表</title>
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
    <h1>当前目录文件列表</h1>
    <ul>' > "$output_file"

# 遍历目标目录中的文件，并创建链接
for file in "$target_dir"/*; do
    filename=$(basename "$file")
    case "$filename" in
        *.html|*.svg)
            echo "<li><a href=\"$filename\">$filename</a></li>" >> "$output_file"
            ;;
    esac
done

# 完成HTML文件
echo '    </ul>
</body>
</html>' >> "$output_file"

