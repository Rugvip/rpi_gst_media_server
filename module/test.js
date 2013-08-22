var net_player = require('./build/Release/net_player');

var player = new net_player.GstPlayer();

player.init();

songs = [
    {
        artist: "Youtube Mixes",
        album: "Hardcore",
        name: "DJ Gonzo I",
    },
    {
        artist: "Youtube Mixes",
        album: "Hardcore",
        name: "DJ Gonzo II",
    },
    {
        artist: "Youtube Mixes",
        album: "Industrial",
        name: "Snow Styler VI",
    },
    {
        artist: "Youtube Mixes",
        album: "Industrial",
        name: "Dark Modulator I",
    },
    {
        artist: "Youtube Mixes",
        album: "Industrial",
        name: "Dark Modulator II",
    },
    {
        artist: "Youtube Mixes",
        album: "Industrial",
        name: "Dark Modulator III",
    },
    {
        artist: "Youtube Mixes",
        album: "Industrial",
        name: "Dark Modulator IV",
    },
    {
        artist: "Youtube Mixes",
        album: "Industrial",
        name: "Dark Modulator V",
    },
    {
        artist: "Youtube Mixes",
        album: "Industrial",
        name: "Dark Modulator VI",
    },
];

player.start();

songs.forEach(function (song) {
    player.queryDuration(song, function (ret, duration) {
        console.log("Song: %j, Duration: %d", ret, duration);
    });
});

console.log("Started");

setInterval(player.iteration, 5);
