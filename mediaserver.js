var _ = require('underscore');
var a_ = require('async');
var fs = require('fs');
var http = require('http');
var net = require('net');
var querystring = require('querystring');
require('colors');

var DIR = '/home/pi/music'
var PORT = 80;

var db = {};

fs.readdir(DIR, function (err, artists) {
    a_.each(artists, function (artist, artistCallback) {
        db[artist] = Object.create(null);
        fs.readdir(DIR + '/' + artist, function (err, albums) {
            a_.each(albums, function (album, albumCallback) {
                fs.readdir(DIR + '/' + artist + '/' + album, function (err, songs) {
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

var server = http.createServer();

server.on('request', function (req, res) {
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
});

server.on('connect', function (req, socket, head) {
    
});

server.listen(PORT)

console.log("Server listening to port %d", PORT);

var staticResponse = function(file, type) {
    return function (req, res) {
        res.writeHead(200, {
            'Content-Type': type
        });
        fs.createReadStream(file).pipe(res);
    }
}

var getTable = {
    '/favicon.ico': staticResponse('www/favicon.ico', 'image/icon'),
    '/': staticResponse('www/index.html', 'text/html'),
    '/style.css': staticResponse('www/style.css', 'text/css'),
    '/script.js': staticResponse('www/script.js', 'application/ecmascript'),
    '/jquery.js': staticResponse('www/jquery-2.0.2.js', 'application/ecmascript'),
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
        }, function (body) {
            console.log("Body: %s", body);
            res.writeHead(200, {
                'Content-Type': 'text/plain'
            });
            res.end("Done");
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
            console.log("Wat: %j", obj);
            res.end("Nope");
        });
    }
}

var mediaServerRequest = function (jsonReq, callback) {
    var body = "";
    var socket = net.createConnection(3264);
    socket.setEncoding('ascii');
    socket.on('connect', function () {
        socket.write(JSON.stringify(jsonReq));
        socket.end();
    });
    socket.on('end', function () {
        callback(body);
    });
    socket.on('data', function (data) {
        body += data;
    });
}
