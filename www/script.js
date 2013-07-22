(function () {
    var req = {
        lol: "wut",
        herp: "derp"
    }
    $.post('http://pi/post', JSON.stringify(req), function (data) {
    });
    $.get('http://pi/get', function (data) {
        console.log(data);
        Object.keys(data).forEach(function (e) {
            var node = document.createElement('div');
            $(node).addClass('artist');
            $(node).html(e);
            $('.frame').append(node);
        });
    });
}());
