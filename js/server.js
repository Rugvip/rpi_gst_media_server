
"use strict";

var http     = require('http'),
    socketio = require('socket.io'),
    _        = require('underscore'),
    a_       = require('async'),
    fs       = require('fs'),
    net      = require('net'),
    music    = require('./music'),
    path     = require('path'),
    spawn    = require('child_process').spawn;

module.exports.start = function () {
    var gst, objs;

    console.log("Starting media gst");

    gst = spawn('./mediaserver', [], {
        detached: true,
    });

    gst.stdout.setEncoding('utf8');
    gst.stderr.setEncoding('utf8');
    gst.stdout.on('data', function (data) {
        console.log("Data: ", data);
    });
    gst.stderr.on('data', function (data) {
        console.log("Err: ", data);
    });

    objs = [
        {
            type: "duration_query",
            artist: "Youtube Mixes",
            album: "Hardcore",
            song: "DJ Gonzo I",
        },
        {
            type: "duration_query",
            artist: "Youtube Mixes",
            album: "Hardcore",
            song: "DJ Gonzo II",
        },
        {
            type: "duration_query",
            artist: "Youtube Mixes",
            album: "Industrial",
            song: "Snow Styler VI",
        },
        {
            type: "duration_query",
            artist: "Youtube Mixes",
            album: "Industrial",
            song: "Dark Modulator I",
        },
        {
            type: "duration_query",
            artist: "Youtube Mixes",
            album: "Industrial",
            song: "Dark Modulator II",
        },
        {
            type: "duration_query",
            artist: "Youtube Mixes",
            album: "Industrial",
            song: "Dark Modulator III",
        },
        {
            type: "duration_query",
            artist: "Youtube Mixes",
            album: "Industrial",
            song: "Dark Modulator IV",
        },
        {
            type: "duration_query",
            artist: "Youtube Mixes",
            album: "Industrial",
            song: "Dark Modulator V",
        },
        {
            type: "duration_query",
            artist: "Youtube Mixes",
            album: "Industrial",
            song: "Dark Modulator VI",
        },
    ];
    setTimeout(function () {
        objs.forEach(function (obj) {
            console.log("WRITIN");
            gst.stdin.write(JSON.stringify(obj));
            gst.stdin.end();
        });
    }, 2000);
};










