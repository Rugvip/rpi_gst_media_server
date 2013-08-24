var gmp3 = require('gmp3');

var player = new gmp3.Player();

player.init([
    "--gst-debug-level=2",
    "--gst-debug-with-color"
]);
player.start();

// songs = [
//     {
//         artist: "Youtube Mixes",
//         album: "Hardcore",
//         name: "DJ Gonzo I",
//     },
//     {
//         artist: "Youtube Mixes",
//         album: "Hardcore",
//         name: "DJ Gonzo II",
//     },
//     {
//         artist: "Youtube Mixes",
//         album: "Industrial",
//         name: "Snow Styler VI",
//     },
//     {
//         artist: "Youtube Mixes",
//         album: "Industrial",
//         name: "Dark Modulator I",
//     },
//     {
//         artist: "Youtube Mixes",
//         album: "Industrial",
//         name: "Dark Modulator II",
//     },
//     {
//         artist: "Youtube Mixes",
//         album: "Industrial",
//         name: "Dark Modulator III",
//     },
//     {
//         artist: "Youtube Mixes",
//         album: "Industrial",
//         name: "Dark Modulator IV",
//     },
//     {
//         artist: "Youtube Mixes",
//         album: "Industrial",
//         name: "Dark Modulator V",
//     },
//     {
//         artist: "Youtube Mixes",
//         album: "Industrial",
//         name: "Dark Modulator VI",
//     },
// ];

// songs.forEach(function (song) {
//     player.queryDuration(song, function (ret, duration) {
//         console.log("Song: %j, Duration: %d", ret, duration);
//     });
// });
