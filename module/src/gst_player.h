#ifndef gst_player_h
#define gst_player_h

#include "../src/common.h"
#include "../src/player.h"

#include <node.h>

class GstPlayer : public node::ObjectWrap {
    public:
        static void Initialize(v8::Handle<v8::Object> exports);

    private:
        GstPlayer();
        ~GstPlayer();

        static v8::Handle<v8::Value> new_instance(const v8::Arguments& args);
        static v8::Handle<v8::Value> init(const v8::Arguments& args);
        static v8::Handle<v8::Value> start(const v8::Arguments& args);
        static v8::Handle<v8::Value> stop(const v8::Arguments& args);
        static v8::Handle<v8::Value> query_duration(const v8::Arguments& args);

        Player *player;
};

#endif /* gst_player_h */
