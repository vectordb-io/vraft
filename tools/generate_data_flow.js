const fs = require('fs');

// 获取输入文件路径
const inputFile = process.argv[2];

if (!inputFile) {
  console.error('请提供文件路径，例如：node process.js ./a.txt');
  process.exit(1);
}

// 读取文件内容
fs.readFile(inputFile, 'utf-8', (err, data) => {
  if (err) {
    console.error('无法读取文件:', err);
    return;
  }

  // 按行分割文件内容
  const lines = data.trim().split('\n');

  lines.forEach(line => {
    const columns = line.trim().split(/\s+/);
    const eventType = columns[1];

    if (eventType === 'event_timer' && columns.length === 6) {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4];
      console.log(addr, '->', addr + ':', event);

    } else if (eventType === 'event_start' && columns.length === 5) {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4];
      console.log(addr, '->', addr + ':', event);

    } else if (
        eventType === 'event_other' && columns.length === 6 &&
        columns[4] === 'propose-value') {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4] + '-' + columns[5];
      console.log(addr, '->', addr + ':', event);

    } else if (
        eventType === 'event_other' && columns.length === 6 &&
        columns[4] === 'become-leader') {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4] + '-' + columns[5];
      console.log(addr, '->', addr + ':', event);

    } else if (
        eventType === 'event_other' && columns.length === 6 &&
        columns[4] === 'increase-term') {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4] + '-' + columns[5];
      console.log(addr, '->', addr + ':', event);

    } else if (
        eventType === 'event_other' && columns.length === 6 &&
        columns[4] === 'step-down') {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4] + '-' + columns[5];
      console.log(addr, '->', addr + ':', event);

    } else if (
        eventType === 'event_other' && columns.length === 5 &&
        columns[4] === 'enable-recv') {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4];
      console.log(addr, '->', addr + ':', event);

    } else if (
        eventType === 'event_other' && columns.length === 5 &&
        columns[4] === 'enable-send') {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4];
      console.log(addr, '->', addr + ':', event);

    } else if (
        eventType === 'event_other' && columns.length === 5 &&
        columns[4] === 'disable-recv') {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4];
      console.log(addr, '->', addr + ':', event);

    } else if (
        eventType === 'event_other' && columns.length === 5 &&
        columns[4] === 'disable-send') {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4];
      console.log(addr, '->', addr + ':', event);

    } else if (
        eventType === 'event_other' && columns.length === 6 &&
        columns[4] === 'leader-transfer') {
      const addr = columns[3].replace(/[:#]/g, '_');
      const addr_to = columns[5].replace(/[:#]/g, '_');
      const event = columns[4] + '-' + addr_to;
      console.log(addr, '->', addr + ':', event);

    } else if (eventType === 'event_stop' && columns.length === 5) {
      const addr = columns[3].replace(/[:#]/g, '_');
      const event = columns[4];
      console.log(addr, '->', addr + ':', event);

    } else if (eventType === 'event_send' && columns.length === 4) {
      try {
        const jsonStr = columns[3];
        const jsonObj = JSON.parse(jsonStr);

        const firstKey = Object.keys(jsonObj)[0];
        const firstValue = jsonObj[firstKey][0];
        const dest = firstValue['dest'].replace(/[:#]/g, '_');
        const src = firstValue['src'].replace(/[:#]/g, '_');
        const uid = firstValue['uid'];
        const term = firstValue['term'];

        // console.log(firstKey, dest, src, uid);
        // console.log(src, "->", dest+":", firstKey+"-"+uid+"-"+"tm:"+term);
        msg_str = src + ' -> ' + dest + ': ' + firstKey + '_' + uid + '_' +
            'tm:' + term;
        if (firstKey === 'request-vote' || firstKey === 'pre-request-vote') {
          const last = jsonObj[firstKey][1]['last'];
          const last_term = jsonObj[firstKey][1]['last-term'];
          const leader_transfer = jsonObj[firstKey][1]['leader-transfer'];
          msg_str += '_last:' + last;
          msg_str += '_ltm:' + last_term;
          msg_str += '_transfer:' + leader_transfer;

        } else if (
            firstKey === 'request-vote-reply' ||
            firstKey === 'pre-request-vote-reply') {
          const grant = jsonObj[firstKey][1]['grant'];
          const log_ok = jsonObj[firstKey][1]['log-ok'];
          msg_str += '_grant:' + grant;
          msg_str += '_logok:' + log_ok;

        } else if (firstKey === 'append-entries') {
          const commit = jsonObj[firstKey][1]['commit'];
          const pre = jsonObj[firstKey][1]['pre'];
          const pre_term = jsonObj[firstKey][1]['pre-term'];
          const entry_count = jsonObj[firstKey][1]['entry-count'];
          msg_str += '_cmt:' + commit;
          msg_str += '_pre:' + pre;
          msg_str += '_ptm:' + pre_term;
          msg_str += '_cnt:' + entry_count;

        } else if (firstKey === 'append-entries-reply') {
          const last = jsonObj[firstKey][1]['last'];
          const success = jsonObj[firstKey][1]['success'];
          msg_str += '_last:' + last;
          msg_str += '_success:' + success;

        } else if (firstKey === 'timeout-now') {
          const last = jsonObj[firstKey][1]['last'];
          const last_term = jsonObj[firstKey][1]['last-term'];
          const force = jsonObj[firstKey][1]['force'];
          msg_str += '_last:' + last;
          msg_str += '_ltm:' + last_term;
          msg_str += '_force:' + force;
        }


        console.log(msg_str);


      } catch (parseError) {
        console.error('JSON解析错误:', parseError);
      }
    } else {
      console.warn('未知或格式不正确的行:', line);
    }
  });
});
