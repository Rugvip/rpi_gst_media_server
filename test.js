var net = require('net');

var contact = {
    type: "play",
    artist: "Daft Punk",
    album: "Random Access Memories",
    song: "Contact",
    time: 0
}

var gonzo = {
    type: "play",
    artist: "Youtube Mixes",
    album: "Hardcore",
    song: "DJ Gonzo II",
    time: 30000
}

var snow = {
    type: "play",
    artist: "Youtube Mixes",
    album: "Industrial",
    song: "Snow Styler VI",
    time: 55000
}

var socket = net.createConnection(3264);
socket.setEncoding('ascii');

socket.on('connect', function () {
    socket.write(JSON.stringify(snow));
    socket.end();
});

socket.on('end', function () {
    console.log("Connection closed");
});

socket.on('data', function (data) {
    console.log("Recieved data: %s", data);
});
