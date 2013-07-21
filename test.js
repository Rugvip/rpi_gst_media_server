var net = require('net');

var testObj = {
    str: "Hello World!",
    number: 1337,
    object: {
        key: "value"
    }
}

var socket = net.createConnection(3264);

socket.on('connect', function () {
    socket.write(JSON.stringify(testObj));
});

socket.on('end', function () {
    console.log("Connection closed");
});

socket.on('data', function (data) {
    console.log("Recieved data: %d", data);
});
