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
    start: int (ms) = 0,
    end: int (ms) = -1,
    transition: int (ms) = 5000,
}

playlist: {
    songs: {
        id: song,
        id: song,
    },
    next: {
        id: id,
        id: id,
        id: id,
        id: id,
        id: null
    }
}
*/

player.start();

socket.on('play', function (song) {
    player.setPlaying(song);
});

player.on('playing', function (id, time) {
    socket.write(id, time);
});

player.on('next', function (id) {
    player.next(playlist[id]);
});

player.on('end', function () {
    socket.write('end');
});

socket.on('playlist', function (list) {
    var playing = player.getPlaying();
    var next = list.next[playing];

    if (next && !player.isNext(next)) {
        player.setNext(next);
    }
});

socket.on('seek', function (time) {
    player.seek(time);
});

/*
player.setSong(song);
var id = player.getPlaying();
player.isPlaying([id]);

player.setNext(song);
var id = player.getNext();
player.isNext(id);
player.hasNext();

player.play();
player.pause();
player.stop();
player.seek(time);

player.getDuration();
player.getPosition();
player.getVolume();
player.getEq();

player.setVolume();
player.setEq();

player.on('start', function());
player.on('playing', function(id, duration, position));
player.on('paused', function(id, position));
player.on('stopped', function(id));
player.on('fade', function(id, next));

*/
