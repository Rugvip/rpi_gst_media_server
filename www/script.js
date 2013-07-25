$(function () {
    var root = Object.create(null);

    var selection = {
        artist: null,
        album: null,
        song: null
    }

    var updateDisplay = function () {
        $('#album_frame').empty();
        $('#song_frame').empty();

        var playStr = "";

        if (selection.artist) {
            playStr += selection.artist.name;
            _.each(root, function (artist) {
                artist.style.backgroundColor = null;
            });
            selection.artist.style.backgroundColor = '#707478';

            _.each(selection.artist.albums, function (album) {
                $('#album_frame').append(album);
                album.style.backgroundColor = null;
            });

            if (selection.album) {
                playStr += ": " + selection.album.name;
                selection.album.style.backgroundColor = '#707478';

                _.each(selection.album.songs, function (song) {
                    $('#song_frame').append(song);
                    song.style.backgroundColor = null;
                });

                if (selection.song) {
                    playStr += " - " + selection.song.name;
                    selection.song.style.backgroundColor = '#707478';
                }
            }
        }

        $('#play_frame').html(playStr);
    };

    $.get('get', function (data) {
        _.each(data, function (albums, artist) {
            var artistDiv = document.createElement('div');
            $(artistDiv).addClass('artist');
            $(artistDiv).html(artist);
            artistDiv.albums = Object.create(null);
            artistDiv.name = artist;
            root[artist] = artistDiv;
            _.each(data[artist], function (songs, album) {
                var albumDiv = document.createElement('div');
                $(albumDiv).addClass('album');
                $(albumDiv).html(album);
                albumDiv.songs = [];
                albumDiv.name = album;
                artistDiv.albums[album] = albumDiv;
                _.each(data[artist][album], function (song) {
                    var songDiv = document.createElement('div');
                    $(songDiv).addClass('song');
                    $(songDiv).html(song);
                    songDiv.name = song;
                    albumDiv.songs.push(songDiv);
                });
            });
            $('#artist_frame').append(artistDiv);
        });
    });

    $('#artist_frame').on('click', '.artist', function (event) {
        if (event.target === selection.artist) {
            console.log("same artist");
            return;
        }
        selection.artist = event.target;
        selection.album = null;
        selection.song = null;
        updateDisplay();
    });

    $('#album_frame').on('click', '.album', function (event) {
        if (event.target === selection.album) {
            console.log("same album");
            return;
        }
        selection.album = event.target;
        selection.song = null;
        updateDisplay();
    });

    $('#song_frame').on('click', '.song', function (event) {
        if (event.target === selection.song) {
            console.log("same song");
            return;
        }
        selection.song = event.target;
        updateDisplay();
    });

    $('#play_frame').on('click', function (event) {
        if (selection.artist && selection.album && selection.song) {
            $.post('post', JSON.stringify({
                action: 'play',
                artist: selection.artist.name,
                album: selection.album.name,
                song: selection.song.name
            }), function (data) {
                $('#play_frame').html(data);
            });
        }
    });
});
