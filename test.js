var net = require('net');

var testObj = {
    action: "start",
}

var socket = net.createConnection(3264);
socket.setEncoding('ascii');

socket.on('connect', function () {
    socket.write(JSON.stringify(testObj));
    socket.end();
});

socket.on('end', function () {
    console.log("Connection closed");
});

socket.on('data', function (data) {
    console.log("Recieved data: %s", data);
});
