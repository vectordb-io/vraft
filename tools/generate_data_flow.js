const fs = require('fs');

// 获取输入文件路径
const inputFile = process.argv[2];

if (!inputFile) {
    console.error("请提供文件路径，例如：node process.js ./a.txt");
    process.exit(1);
}

// 读取文件内容
fs.readFile(inputFile, 'utf-8', (err, data) => {
    if (err) {
        console.error("无法读取文件:", err);
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
            console.log(addr, "->", addr+":", event);
        } else if (eventType === 'event_start' && columns.length === 5) {
            const addr = columns[3].replace(/[:#]/g, '_');
            const event = columns[4];
            console.log(addr, "->", addr+":", event);
        } else if (eventType === 'event_stop' && columns.length === 5) {
            const addr = columns[3].replace(/[:#]/g, '_');
            const event = columns[4];
            console.log(addr, "->", addr+":", event);
        } else if (eventType === 'event_send' && columns.length === 4) {
            try {
                const jsonStr = columns[3];
                const jsonObj = JSON.parse(jsonStr);
                
                const firstKey = Object.keys(jsonObj)[0];
                const firstValue = jsonObj[firstKey][0];
                const dest = firstValue['dest'].replace(/[:#]/g, '_');
                const src = firstValue['src'].replace(/[:#]/g, '_');
                const uid = firstValue['uid'].replace(/[:#]/g, '_');

                //console.log(firstKey, dest, src, uid);
                console.log(src, "->", dest+":", firstKey+"-"+uid);

            } catch (parseError) {
                console.error("JSON解析错误:", parseError);
            }
        } else {
            console.warn("未知或格式不正确的行:", line);
        }
    });
});
