var gmp3 = require('gmp3');

gmp3.init([
    "--gst-debug-level=2",
    "--gst-debug-with-color"
]);

var player = new gmp3.Player();

// load songs

player.on('start', function (str) {
    console.log("Stared: " + str);
});

/*
song: {
    id: string,
    path: string,
    start: int (ms) = -1,
    end: int (ms) = -1,
}

playlist: {
    songs: {
        id: song,
        id: song,
    },
    transitions: {
        id: {
            next: id,
            transition: time,
        },
        id: {
            next: id,
            transition: time,
        },
        id: {
            next: null,
            transition: fade time,
        }
    }
}
*/

player.start();

socket.on('play', function (song) {
    player.play(song);
});

player.on('playing', function (id) {
    socket.write(songs[id]);
});

player.on('next', function (id) {
    player.next(playlist[id]);
});

player.on('end', function () {
    socket.write('end');
});

var id = player.getPlaying();
var id = player.getNext();

player.isPlaying([id]);
player.hasNext([id]);
