{
    "targets": [
        {
            "target_name": "test",
            "sources": [
                "test.cc",
                "../src/gst_utils.c",
                "../src/handler.c",
                "../src/io.c",
                "../src/jsongen.c",
                "../src/jsonio.c",
                "../src/jsonparse.c",
                "../src/main.c",
                "../src/player.c",
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
