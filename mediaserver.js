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

server.on('request', function (request, response) {
    console.log(request);
    var body = JSON.stringify(db);
    response.writeHead(200, {
        'Content-Length': body.length,
        'Content-Type': 'text/json'
    })
    response.end(body);
});

server.on('connect', function (request, socket, head) {
    
});

server.listen(PORT)

console.log("Server listening to port %d", PORT);

