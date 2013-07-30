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
    song: "DJ Gonzo 1.mp3",
    time: 0
}

var snow = {
    type: "play",
    artist: "Youtube Mixes",
    album: "Industrial",
    song: "Snow Styler VI",
    time: 0
}

var socket = net.createConnection(3264);
socket.setEncoding('ascii');

socket.on('connect', function () {
    socket.write(JSON.stringify(gonzo));
    socket.end();
});

socket.on('end', function () {
    console.log("Connection closed");
});

socket.on('data', function (data) {
    console.log("Recieved data: %s", data);
});
