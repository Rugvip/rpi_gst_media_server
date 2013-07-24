var fs = require('fs');
var spawn = require('child_process').spawn;
require('colors');

console.log("Running!".green);

var remake = function() {
    var make = null, server = null;
    return function () {
        console.log("Make triggered".blue);

        if (make) {
            console.log("Make already running, killing".cyan);
            make.kill();
        }

        make = spawn('make', [], {
            detached: true,
            stdio: ['ignore', process.stdout, process.stderr]
        });

        make.on('exit', function (code, signal) {
            make = null;
            if (!code && !signal) {
                console.log("Make completed".green);
                if (server) {
                    console.log("Server already running, killing".cyan);
                    server.kill();
                }
                server = spawn('./mediastreamer', [], {
                    detached: true,
                    stdio: ['ignore', process.stdout, process.stderr]
                });
            } else {
                console.log("Make completed (%d) [%s]".red, code, signal);
            }
        });
    };
}();

var watch = fs.watch('src', function (event, filename) {
    trigger();
});

var trigger = function() {
    var timeout = false;
    return function() {
        if (!timeout) {
            timeout = true;
            setTimeout(function() {
                timeout = false;
                remake();
            }, 200);
        }
    }
}();
