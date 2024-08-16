const http = require('http');
const fs = require('fs');
const path = require('path');

if (process.argv.length !== 3) {
    console.error('Usage: node remu_server.js <directory_path>');
    process.exit(1);
}

const directoryPath = process.argv[2];
const port = 3000;

const server = http.createServer((req, res) => {
    if (req.url === '/') {
        fs.readdir(directoryPath, (err, files) => {
            if (err) {
                res.writeHead(500, {'Content-Type': 'text/plain'});
                res.end('Unable to scan directory');
                return;
            }

            const htmlFiles = files.filter(file => file.endsWith('.html'));
            const svgFiles = files.filter(file => file.endsWith('.svg'));
            const otherFiles = files.filter(file =>
                file.endsWith('.sm') || file.endsWith('.log') || file.endsWith('.message') || file.endsWith('.global')
            );

            const generateLinks = (files) => {
                return files.map(file => `<a href="${file}" target="_blank">${file}</a>`).join('<br>');
            };

            const htmlContent = `
                <!DOCTYPE html>
                <html lang="en">
                <head>
                    <meta charset="UTF-8">
                    <meta name="viewport" content="width=device-width, initial-scale=1.0">
                    <title>(REMU)Raft Emulator</title>
                    <link rel="stylesheet" type="text/css" href="/remu_web.css">
                </head>
                <body>
                    <table><tr><td class="banner">(REMU)Raft Emulator</td></tr></table>

                    <div class="html-files">
                    <h2>Generated Files</h2>
                    <ul><li>${generateLinks(htmlFiles)}</li></ul>
                    </div>

                    <div class="html-files">
                    <h2>Puml Message Flow</h2>
                    <ul><li>${generateLinks(svgFiles)}</li></ul>
                    </div>

                    <div class="other-files">
                    <h2>Other Files</h2>
                    <ul><li>${generateLinks(otherFiles)}</li></ul>
                    </div>
                </body>
                </html>
            `;

            res.writeHead(200, {'Content-Type': 'text/html'});
            res.end(htmlContent);
        });
    } else if (req.url === '/remu_web.css') {
        const cssPath = path.join(__dirname, 'remu_web.css');
        fs.readFile(cssPath, (err, data) => {
            if (err) {
                res.writeHead(404, {'Content-Type': 'text/plain'});
                res.end('CSS file not found');
                return;
            }
            res.writeHead(200, {'Content-Type': 'text/css'});
            res.end(data);
        });
    } else if (req.url === '/a.css') {
        const cssPath = path.join(__dirname, 'a.css');
        fs.readFile(cssPath, (err, data) => {
            if (err) {
                res.writeHead(404, {'Content-Type': 'text/plain'});
                res.end('CSS file not found');
                return;
            }
            res.writeHead(200, {'Content-Type': 'text/css'});
            res.end(data);
        });
    } else if (req.url === '/b.css') {
        const cssPath = path.join(__dirname, 'b.css');
        fs.readFile(cssPath, (err, data) => {
            if (err) {
                res.writeHead(404, {'Content-Type': 'text/plain'});
                res.end('CSS file not found');
                return;
            }
            res.writeHead(200, {'Content-Type': 'text/css'});
            res.end(data);
        });
    } else {
        const filePath = path.join(directoryPath, req.url);
        fs.access(filePath, fs.constants.F_OK, (err) => {
            if (err) {
                res.writeHead(404, {'Content-Type': 'text/plain'});
                res.end('Not Found');
                return;
            }

            fs.readFile(filePath, (err, data) => {
                if (err) {
                    res.writeHead(500, {'Content-Type': 'text/plain'});
                    res.end('Internal Server Error');
                    return;
                }

                let contentType = 'application/octet-stream';
                if (filePath.endsWith('.html')) {
                    contentType = 'text/html';
                } else if (filePath.endsWith('.svg')) {
                    contentType = 'image/svg+xml';
                } else if (filePath.endsWith('.sm') || filePath.endsWith('.log') || filePath.endsWith('.message') || filePath.endsWith('.global')) {
                    contentType = 'text/plain';
                }

                res.writeHead(200, {'Content-Type': contentType});
                res.end(data);
            });
        });
    }
});

server.listen(port, '0.0.0.0', () => {
    console.log(`Web server running at http://0.0.0.0:${port}`);
});
