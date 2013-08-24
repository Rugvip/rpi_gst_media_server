var gcontext = require('gcontext');

var mp3 = require('../build/Release/net_player.node');
var events = require('events');

var Player = mp3.GstPlayer;

inherits(Player, events.EventEmitter);

Player.prototype._extend = function () {
    events.EventEmitter.call(this);
};

module.exports.Player = Player;

module.exports.init = function () {
    mp3.init.apply(mp3, arguments);
    gcontext.init();
};

function inherits(target, source) {
  for (var k in source.prototype)
    target.prototype[k] = source.prototype[k];
}
