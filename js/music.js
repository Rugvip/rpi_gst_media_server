"use strict";

var fs     = require('fs'),
    a_     = require('async'),
    events = require('events'),
    db     = Object.create(null);

var MUSIC_DIR = process.env.HOME + '/Music';

var emitter = new events.EventEmitter();

module.exports = emitter;

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
        emitter.db = db;
        emitter.emit('load', err);
    });
});
