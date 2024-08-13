const fs = require('fs');
const readline = require('readline');

// 获取命令行参数（除去前两个元素）
const args = process.argv.slice(2);

// 检查参数数量
if (args.length !== 2) {
  console.error(
      'Usage: ' + process.argv[0] + ' ' + process.argv[1] +
      ' ./remu.log.sm ./html/node.html.body2');
  process.exit(1);
}

// 解构赋值来获取 input 和 output 文件路径
const [inputFile, outputFile] = args;

// 创建读取流
const readStream = fs.createReadStream(inputFile);

// 创建写入流
const writeStream = fs.createWriteStream(outputFile, {flags: 'w+'});

// 使用 readline 处理逐行读取
const rl = readline.createInterface({
  input: readStream,
  crlfDelay: Infinity  // 此设置支持 Windows (CRLF) 和 UNIX (LF) 系统的换行模式
});

var state = 'finish';
var begin_json_obj = {}

const td_str = '\t\t<td>';
const td_change_str = '\t\t<td class="change">';
const td3_str = '\t\t<td colspan="3">'
const td3_change_str = '\t\t<td class="change" colspan="3">'
const td_end_str = '</td>\n';

rl.on('line', (line) => {
  // console.log(line);  // 在控制台打印每一行
  // writeStream.write(line + '\n');  // 将每一行写入文件，添加换行符

  if (state == 'finish') {
    if (line != '') {
      writeStream.write('<table>\n');

      writeStream.write('\t<tr>\n');
      writeStream.write('\t\t<td class="table-title" colspan="5">');
      writeStream.write(line);
      writeStream.write('</td>\n');
      writeStream.write('\t</tr>\n');

      state = 'data';
    }

  } else if (state == 'data') {
    if (line != '') {
      var parts = line.split(/\s+/);
      var unique_id = parts[0];
      var event_name = parts[1];

      if (event_name == 'state_begin' || event_name == 'state_end') {
        writeStream.write('\t<tr>\n');
        writeStream.write('\t\t<td class="event" colspan="5">');
        writeStream.write(event_name);
        writeStream.write(td_end_str);
        writeStream.write('\t</tr>\n');

        var json_str = parts[3];
        var json_obj = JSON.parse(json_str);

        // generate one node
        // line 1
        writeStream.write('\t<tr>\n');

        writeStream.write('\t\t<td class="node">');
        var keys = Object.keys(json_obj);
        raftid = keys[0];
        writeStream.write(raftid)
        writeStream.write(td_end_str);

        // when begin, save
        if (event_name == 'state_begin') {
          begin_json_obj = json_obj;
        }

        var raft_state = json_obj[raftid][0];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            raft_state != begin_json_obj[raftid][0]) {
          writeStream.write(td_change_str);
        } else {
          writeStream.write(td_str);
        }
        writeStream.write(raft_state)
        writeStream.write(td_end_str);

        writeStream.write(td_str);
        var raft_ptr = json_obj[raftid][1][1];
        writeStream.write(raft_ptr);
        writeStream.write(td_end_str);

        var pre_voting = json_obj[raftid][1][0][2]['pre-voting'];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            pre_voting != begin_json_obj[raftid][1][0][2]['pre-voting']) {
          writeStream.write(td_change_str);
        } else {
          writeStream.write(td_str);
        }
        writeStream.write('"pre-voting":' + pre_voting);
        writeStream.write(td_end_str);

        var transfer = json_obj[raftid][1][0][2]['transfer'];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            transfer != begin_json_obj[raftid][1][0][2]['transfer']) {
          writeStream.write(td_change_str);
        } else {
          writeStream.write(td_str);
        }
        writeStream.write('"transfer":' + transfer);
        writeStream.write(td_end_str);



        writeStream.write('\t</tr>\n');

        // line 2
        writeStream.write('\t<tr>\n');

        var term = json_obj[raftid][1][0][0]['term'];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            term != begin_json_obj[raftid][1][0][0]['term']) {
          writeStream.write(td_change_str);
        } else {
          writeStream.write(td_str);
        }
        writeStream.write('"term":' + term);
        writeStream.write(td_end_str);

        var vote = json_obj[raftid][1][0][0]['vote'];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            vote != begin_json_obj[raftid][1][0][0]['vote']) {
          writeStream.write(td_change_str);
        } else {
          writeStream.write(td_str);
        }
        writeStream.write('"vote":' + vote);
        writeStream.write(td_end_str);

        var log = json_obj[raftid][1][0][1]['log'];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            JSON.stringify(log) !=
                JSON.stringify(begin_json_obj[raftid][1][0][1]['log'])) {
          writeStream.write(td3_change_str);
        } else {
          writeStream.write(td3_str);
        }
        writeStream.write('"log":' + JSON.stringify(log));
        writeStream.write(td_end_str);

        writeStream.write('\t</tr>\n');

        // line 3
        writeStream.write('\t<tr>\n');

        var apply = json_obj[raftid][1][0][2]['apply'];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            apply != begin_json_obj[raftid][1][0][2]['apply']) {
          writeStream.write(td_change_str);
        } else {
          writeStream.write(td_str);
        }
        writeStream.write('"apply":' + apply);
        writeStream.write(td_end_str);

        var cmt = json_obj[raftid][1][0][2]['cmt'];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            cmt != begin_json_obj[raftid][1][0][2]['cmt']) {
          writeStream.write(td_change_str);
        } else {
          writeStream.write(td_str);
        }
        writeStream.write('"cmt":' + cmt);
        writeStream.write(td_end_str);

        var elect_ms = json_obj[raftid][1][0][2]['elect_ms'];
        /*
                if ((event_name == "state_end") &&
           (Object.keys(begin_json_obj).length != 0) && elect_ms !=
           begin_json_obj[raftid][1][0][2]["elect_ms"]) {
                    writeStream.write(td_change_str);
                } else {
                    writeStream.write(td_str);
                }
        */
        writeStream.write(td_str);
        writeStream.write('"elect_ms":' + JSON.stringify(elect_ms));
        writeStream.write(td_end_str);

        var leader = json_obj[raftid][1][0][2]['leader'];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            leader != begin_json_obj[raftid][1][0][2]['leader']) {
          writeStream.write(td_change_str);
        } else {
          writeStream.write(td_str);
        }
        writeStream.write('"leader":' + leader);
        writeStream.write(td_end_str);

        var run = json_obj[raftid][1][0][2]['run'];
        if ((event_name == 'state_end') &&
            (Object.keys(begin_json_obj).length != 0) &&
            run != begin_json_obj[raftid][1][0][2]['run']) {
          writeStream.write(td_change_str);
        } else {
          writeStream.write(td_str);
        }
        writeStream.write('"run":' + run);
        writeStream.write(td_end_str);

        writeStream.write('\t</tr>\n');

        // line 4
        if (Object.keys(json_obj[raftid][1][0]).length >= 4) {
          var peers = Object.keys(json_obj[raftid][1][0][3]);
          for (let i = 0; i < peers.length; i++) {
            var peer = peers[i];
            writeStream.write('\t<tr>\n');

            writeStream.write(td_str);
            writeStream.write(peer);
            writeStream.write(td_end_str);

            var match = json_obj[raftid][1][0][3][peer][0]['match'];
            if ((event_name == 'state_end') &&
                (Object.keys(begin_json_obj).length != 0) &&
                match != begin_json_obj[raftid][1][0][3][peer][0]['match']) {
              writeStream.write(td_change_str);
            } else {
              writeStream.write(td_str);
            }
            writeStream.write('"match":' + match);
            writeStream.write(td_end_str);

            var next = json_obj[raftid][1][0][3][peer][0]['next'];
            if ((event_name == 'state_end') &&
                (Object.keys(begin_json_obj).length != 0) &&
                next != begin_json_obj[raftid][1][0][3][peer][0]['next']) {
              writeStream.write(td_change_str);
            } else {
              writeStream.write(td_str);
            }
            writeStream.write('"next":' + next);
            writeStream.write(td_end_str);

            var done = json_obj[raftid][1][0][3][peer][1]['done'];
            if ((event_name == 'state_end') &&
                (Object.keys(begin_json_obj).length != 0) &&
                done != begin_json_obj[raftid][1][0][3][peer][1]['done']) {
              writeStream.write(td_change_str);
            } else {
              writeStream.write(td_str);
            }
            writeStream.write('"done":' + done);
            writeStream.write(td_end_str);

            var grant = json_obj[raftid][1][0][3][peer][1]['grant'];
            if ((event_name == 'state_end') &&
                (Object.keys(begin_json_obj).length != 0) &&
                grant != begin_json_obj[raftid][1][0][3][peer][1]['grant']) {
              writeStream.write(td_change_str);
            } else {
              writeStream.write(td_str);
            }
            writeStream.write('"grant":' + grant);
            writeStream.write(td_end_str);

            writeStream.write('\t</tr>\n');
          }
        }

      } else {
        var tmp_parts = line.split(': ');
        var event_desc = tmp_parts[1];

        writeStream.write('\t<tr>\n');
        writeStream.write('\t\t<td class="event">');
        writeStream.write(event_name);
        writeStream.write(td_end_str);

        writeStream.write('\t\t<td class="event" colspan="4">');
        writeStream.write(event_desc);
        writeStream.write(td_end_str);
        writeStream.write('\t</tr>\n');
      }

    } else {
      state = 'finish';
    }
  }
});

rl.on('close', () => {
  // console.log('File reading and writing completed.');

  writeStream.write('</table>\n');

  writeStream.end();  // 关闭写入流
});

// 错误处理
readStream.on('error', error => {
  console.error('Error reading the input file:', error);
});
writeStream.on('error', error => {
  console.error('Error writing to the output file:', error);
});