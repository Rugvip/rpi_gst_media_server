var net_player = require('./build/Release/net_player');

var player = new net_player.Player();

player.init();
player.start();
