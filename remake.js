"use strict";

var fs = require('fs');
var spawn = require('child_process').spawn;
var a_ = require('async');
var _ = require('underscore');
require('colors');

console.log("Running!".green);

try {

var remake = (function () {
    var make = null, server = null;
    return function () {
        console.log("Make triggered".blue);

        if (make) {
            console.log("Make already running, killing".cyan);
            make.kill();
        }

        make = spawn('make', [], {
            detached: true,
            stdio: ['ignore']
        });

        make.stdout.pipe(process.stdout);
        make.stderr.pipe(process.stderr);
/*        make.stderr.on('data', function (d) {
            console.log("Dataz: " + d);
        });
*/
        make.on('exit', function (code, signal) {
            make = null;
            if (!code && !signal) {
                console.log("Make completed".green);
                if (server) {
                    console.log("Server already running, killing".cyan);
                    server.kill();
                }
                server = spawn('./mediaserver', [], {
                    detached: true,
                    stdio: ['ignore', process.stdout, process.stderr]
                });
            } else {
                console.log("Make completed (%d) [%s]".red, code, signal);
            }
        });
    };
}());

var trigger = (function () {
    var timeout = false;
    return function () {
        if (!timeout) {
            timeout = true;
            setTimeout(function () {
                timeout = false;
                remake();
            }, 200);
        }
    };
}());

/*var watch = fs.watch('src', function (event, filename) {
    trigger();
});*/

fs.readdir('src', function (err, files) {
    files = _.map(files, function (file) {
        return 'src/' + file;
    });
    _.each(files, function (file) {
        console.log("Watch: " + file);
        do {
            var e = null;
            try {
                var watch = fs.watch('src', function (event, filename) {
            try {
                    trigger();
            } catch (e) {
                console.log("WUT: " + e);
            }
                });
            } catch (e) {
                console.log("WAT: " + e);
            }
        } while (e !== null);
    });
});

} catch (e) {
    console.log("I SAW A FISH: " + e);
}

/*var watch = function (file, callback) {
    console.log("Faul: " + file);
    var ret = fs.watch(file, function (event, filename) {
        trigger();
    });
    callback(null, ret);
};

fs.readdir('src', function (err, files) {
    files = _.map(files, function (file) {
        return 'src/' + file;
    });
    console.log("Read: " + files);
    a_.mapSeries(files, watch, function (err, list) {
        console.log("Wat: " + list);
    });
});*/
