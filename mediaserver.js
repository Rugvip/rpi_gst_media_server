var _ = require('underscore');
var a_ = require('async');
var fs = require('fs');
var http = require('http');
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
    if (responseTable[req.url]) {
        responseTable[req.url](req, res);
    } else {
        var notFound = "Not found";
        res.writeHead(404, {
            'Content-Length': notFound.length,
            'Content-Type': 'text/plain'
        });
        res.end(notFound);
    }
});

server.on('connect', function (req, socket, head) {
    
});

server.listen(PORT)

console.log("Server listening to port %d", PORT);

var responseTable = {
    '/favicon.ico': function (req, res) {
        res.writeHead(200, {
            'Content-Type': 'image/icon'
        })
        var stream = fs.createReadStream('www/favicon.ico');
        stream.pipe(res);
        // stream.on('end', function () {
        //     res.end();
        // });
    },
    '/get': function (req, res) {
        var body = JSON.stringify(db);
        res.writeHead(200, {
            'Content-Length': body.length,
            'Content-Type': 'text/json'
        });
        res.end(body);
    },
    '/': function (req, res) {
        res.writeHead(200, {
            'Content-Type': 'text/html'
        });
        var stream = fs.createReadStream('www/index.html');
        stream.pipe(res);
    }
}
