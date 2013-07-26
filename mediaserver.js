var app  = require('http').createServer(requestHandler)
  , io   = require('socket.io').listen(app)
  , _    = require('underscore')
  , a_   = require('async')
  , fs   = require('fs')
  , net  = require('net')
  , path = require('path')

require('colors');

var MUSIC_DIR = '/home/pi/music'
var PORT = 80;

var db = {};

app.listen(PORT);

console.log("Server listening to port %d", PORT);

fs.readdir(MUSIC_DIR, function (err, artists) {
    a_.each(artists, function (artist, artistCallback) {
        db[artist] = Object.create(null);
        fs.readdir(MUSIC_DIR + '/' + artist, function (err, albums) {
            a_.each(albums, function (album, albumCallback) {
                fs.readdir(MUSIC_DIR + '/' + artist + '/' + album, function (err, songs) {
                    db[artist][album] = songs;
                    albumCallback(err);
                });
            }, function (err) {
                artistCallback(err);
            });
        });
    }, function (err) {
        if (err) {
            console.err("Error reading media files %j", err);
        }
        console.log("Done reading media files");
    });
});

function requestHandler(req, res) {
    console.log(req.url);
    if (req.method === 'GET') {
        if (getTable[req.url]) {
            getTable[req.url](req, res);
        } else {
            res.writeHead(404, {
                'Content-Type': 'text/plain'
            });
            res.end("Not found");
        }
    }
    if (req.method === 'POST') {
        if (postTable[req.url]) {
            postTable[req.url](req, res);
        } else {
            res.writeHead(404);
            res.end();
        }
    }
};

var staticResponse = function(file, type) {
    return function (req, res) {
        res.writeHead(200, {
            'Content-Type': type
        });
        fs.createReadStream(__dirname + '/www/' + file).pipe(res);
    }
}

var getTable = {
    '/favicon.ico': staticResponse('img/favicon.ico', 'image/icon'),
    '/': staticResponse('index.html', 'text/html'),
    '/style.css': staticResponse('css/style.css', 'text/css'),
    '/script.js': staticResponse('js/script.js', 'application/ecmascript'),
    '/jquery.js': staticResponse('js/jquery-2.0.2.js', 'application/ecmascript'),
    '/underscore.js': staticResponse('js/underscore.js', 'application/ecmascript'),
    '/bootstrap.js': staticResponse('js/bootstrap.js', 'application/ecmascript'),
    '/get': function (req, res) {
        var body = JSON.stringify(db);
        res.writeHead(200, {
            'Content-Length': body.length,
            'Content-Type': 'text/json'
        });
        res.end(body);
    },
    '/play': function (req, res) {
        mediaServerRequest({
            action: 'start',
            file: "/home/pi/music/Daft Punk/Random Access Memories/Contact"
        });
    },
    '/seek': function (req, res) {
        mediaServerRequest({
            action: 'stop'
        });
    }
}

var postTable = {
    '/post': function (req, res) {
        var body = "";
        req.on('data', function (data) {
            body += data;
        });
        req.on('end', function () {
            var obj = JSON.parse(body);
            if (mediaRequestActionTable[obj.action]) {
                mediaRequestActionTable[obj.action](req, res, obj);
            }
        });
    }
}

var mediaRequestActionTable = {
    'play': function (req, res, obj) {
        if (!obj.artist || ! obj.album || !obj.song) {
            return;
        }
        var file = path.join(MUSIC_DIR, obj.artist, obj.album, obj.song);
        mediaServerRequest({
            action: 'start',
            file: file
        });
    }
}

// Connect to media server an set up event handlers
var mediaServer = function() {
    var socket = net.createConnection(3264);
    socket.setEncoding('ascii');

    socket.on('data', function (data) {
        console.log(data);
        io.sockets.emit('playback', data);
    });
    return socket;
}();

var mediaServerRequest = function (jsonReq) {
    mediaServer.write(JSON.stringify(jsonReq));
};

io.sockets.on('connection', function (socket) {
    console.log("wat");
    socket.emit('msg', "Hello thar!");
    socket.on('response', function (response) {
        console.log("Response " + response);
    });
});
