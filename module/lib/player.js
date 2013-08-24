var gcontext = require('gcontext');

var mp3 = require('../build/Release/net_player.node');

function Player()
{
    this.player = new mp3.GstPlayer();
}

Player.prototype.start = function()
{
    this.player.start();
}

Player.prototype.init = function()
{
    this.player.init();
}

module.exports.Player = Player;

module.exports.init = function () {
    mp3.init.apply(mp3, arguments);
    gcontext.init();
};
