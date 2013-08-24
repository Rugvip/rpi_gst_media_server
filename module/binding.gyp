{
    "targets": [
        {
            "target_name": "net_player",
            "sources": [
                "src/net_player.cc",
                "src/gst_player.cc",
                "src/gst_utils.c",
                "src/main.c",
                "src/player.c",
            ],
            "cflags": [
                "<!@(pkg-config --cflags glib-2.0 json-glib-1.0 gstreamer-1.0 gio-unix-2.0)",
            ],
            "ldflags": [
                "<!@(pkg-config --libs-only-L --libs-only-other glib-2.0 gstreamer-1.0 json-glib-1.0 gio-unix-2.0)",
            ],
            "libraries": [
                "<!@(pkg-config --libs-only-l glib-2.0 gstreamer-1.0 json-glib-1.0 )",
            ],
        }
    ]
}
