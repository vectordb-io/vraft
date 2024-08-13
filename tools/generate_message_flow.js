const fs = require('fs');
const path = require('path');

// 获取命令行参数
const args = process.argv.slice(2);
if (args.length < 1) {
  console.error('Usage: node generate_message_flow.js <input_file>');
  process.exit(1);
}

const inputFile = args[0];

// 固定项目边距和节点间距
const margin = { top: 80, left: 150, bottom: 40 };
const nodeSpacing = 200;
const messageSpacing = 30;

// 读取数据文件
const data = fs.readFileSync(inputFile, 'utf-8').trim();

// 解析数据
const messages = data.split('\n').map(line => {
  const [sender, receiver, message] = line.split(' ');
  return { sender, receiver, message };
});

// 获取所有唯一的节点
const nodes = Array.from(new Set(messages.reduce((acc, msg) => {
  acc.push(msg.sender, msg.receiver);
  return acc;
}, [])));

// 计算画布高度和宽度
const svgHeight = messages.length * messageSpacing + margin.top + margin.bottom;
const svgWidth = nodes.length * nodeSpacing + margin.left * 2;

// 生成HTML内容
const generateHtml = (nodes, messages) => `
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Message Flow Diagram</title>
  <style>
    .node-line {
      stroke: #000;
      stroke-width: 1.5px;
    }
    .link {
      fill: none;
      stroke: #000;
      stroke-width: 1.5px;
      marker-end: url(#end-arrow);
    }
    .loop-link {
      fill: none;
      stroke: #000;
      stroke-width: 1.5px;
      marker-end: url(#end-arrow);
    }
    .label {
      font: 12px sans-serif;
    }
    .node-label {
      font: 14px sans-serif;
      fill: #000;
    }
    .node-bg {
      fill: yellow;
    }
  </style>
  <script src="https://d3js.org/d3.v6.min.js"></script>
</head>
<body>
  <svg width="${svgWidth}" height="${svgHeight}"></svg>
  <script>
    const nodes = ${JSON.stringify(nodes.map((node, idx) => ({ id: node, index: idx })))};
    const messages = ${JSON.stringify(messages.map((msg, idx) => ({
      source: msg.sender,
      target: msg.receiver,
      message: msg.message,
      index: idx
    })))};

    const svg = d3.select("svg");

    svg.append('defs').append('marker')
        .attr('id', 'end-arrow')
        .attr('viewBox', '0 -5 10 10')
        .attr('refX', 10)
        .attr('refY', 0)
        .attr('markerWidth', 6)
        .attr('markerHeight', 6)
        .attr('orient', 'auto')
        .append('path')
        .attr('d', 'M0,-5L10,0L0,5')
        .attr('fill', '#000');

    nodes.forEach((node, i) => {
      const x = ${margin.left} + i * ${nodeSpacing};
      const y = ${margin.top} - 40;

      svg.append("line")
         .attr("x1", x)
         .attr("y1", ${margin.top})
         .attr("x2", x)
         .attr("y2", ${svgHeight} - ${margin.bottom})
         .attr("stroke", "#000")
         .attr("class", "node-line");

      // 计算文本宽度并设置背景矩形宽度
      const text = svg.append("text")
         .attr("x", x)
         .attr("y", y)
         .attr("text-anchor", "middle")
         .attr("class", "node-label")
         .text(node.id);

      const bbox = text.node().getBBox();
      const padding = 5;
      
      // 更新矩形背景大小
      svg.insert("rect", "text")
         .attr("x", bbox.x - padding)
         .attr("y", bbox.y - padding)
         .attr("width", bbox.width + 2 * padding)
         .attr("height", bbox.height + 2 * padding)
         .attr("class", "node-bg");     
    });

    messages.forEach((msg, idx) => {
      const sourceIndex = nodes.findIndex(node => node.id === msg.source);
      const targetIndex = nodes.findIndex(node => node.id === msg.target);

      const y = ${margin.top} + idx * ${messageSpacing};
      
      if (sourceIndex === targetIndex) {
        // 自循环消息
        const x = ${margin.left} + sourceIndex * ${nodeSpacing};
        svg.append("path")
           .attr("d", \`M\${x},\${y} C\${x - 20},\${y - 20} \${x + 20},\${y - 20} \${x},\${y}\`)
           .attr("class", "loop-link");

        svg.append("text")
           .attr("x", x + 25)  // 将x坐标向右移以避免与其它生命线重叠
           .attr("y", y - 10)  // 调整 y 坐标，使文本位于弧线右侧
           .attr("text-anchor", "start")
           .attr("class", "label")
           .text(msg.message);

      } else {
        // 正常消息
        svg.append("line")
           .attr("x1", ${margin.left} + sourceIndex * ${nodeSpacing})
           .attr("y1", y)
           .attr("x2", ${margin.left} + targetIndex * ${nodeSpacing})
           .attr("y2", y)
           .attr("class", "link");

        svg.append("text")
           .attr("x", (${margin.left} + sourceIndex * ${nodeSpacing} + ${margin.left} + targetIndex * ${nodeSpacing}) / 2)
           .attr("y", y - 5)
           .attr("text-anchor", "middle")
           .attr("class", "label")
           .text(msg.message);
      }
    });
  </script>
</body>
</html>
`;

// 生成HTML文件
console.log(generateHtml(nodes, messages));
