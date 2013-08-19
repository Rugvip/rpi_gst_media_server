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
    var gst, obj;

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


    obj = {
        type: "volume",
        volume: 1.5
    };
    gst.stdin.write(JSON.stringify(obj));
};










