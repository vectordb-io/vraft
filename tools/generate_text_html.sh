#!/bin/sh

# 检查是否提供了正确的文件路径
if [ ! -f "$1" ]; then
  echo "Usage: sh generate_log_html.sh [path_to_log_file]"
  exit 1
fi

LOG_FILE="$1"

# print the beginning of the HTML file
echo "<!DOCTYPE html>"
echo "<html lang=\"en\">"
echo "<head>"
echo "    <meta charset=\"UTF-8\">"
echo "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
echo "    <title>Log File</title>"
echo "    <style>"
echo "        body {"
echo "            font-family: Arial, sans-serif;"
echo "            line-height: 1.6;"
echo "            padding: 20px;"
echo "        }"
echo "        .log-line {"
echo "            white-space: pre;"
echo "        }"
echo "    </style>"
echo "</head>"
echo "<body>"
echo "    <h1>Log File</h1>"
echo "    <pre>"

# print each line of log file wrapped in a span
while IFS= read -r line
do
  echo "        <span class=\"log-line\">$(echo "$line" | sed 's/&/\&amp;/g; s/</\&lt;/g; s/>/\&gt;/g; s/"/\&quot;/g; s/'"'"'/\&#39;/g')</span>"
done < "$LOG_FILE"

# print the end of the HTML file
echo "    </pre>"
echo "</body>"
echo "</html>"
