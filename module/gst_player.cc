#define BUILDING_NODE_EXTENSION
#include <uv.h>
#include <node.h>

#include "gst_player.h"

#include "../src/gst_utils.h"

using namespace v8;

GstPlayer::GstPlayer() {
    this->player = player_alloc();
};
GstPlayer::~GstPlayer() {};

Handle<Value> GstPlayer::new_instance(const Arguments& args)
{
    HandleScope scope;

    GstPlayer* obj = new GstPlayer();

    // Local<String> str = args[0]->ToString();
    // obj->counter_ = args[0]->IsUndefined() ? 0 : args[0]->NumberValue();
    obj->Wrap(args.This());

    return args.This();
}

Handle<Value> GstPlayer::init(const Arguments& args)
{
    HandleScope scope;

    g_print("Initializing player\n");

    GstPlayer* obj = ObjectWrap::Unwrap<GstPlayer>(args.This());
    player_init(obj->player);

    return scope.Close(Undefined());
}

Handle<Value> GstPlayer::start(const Arguments& args)
{
    HandleScope scope;

    g_print("Starting player\n");

    GstPlayer* obj = ObjectWrap::Unwrap<GstPlayer>(args.This());

    player_start(obj->player);

    return scope.Close(Undefined());
}

Handle<Value> GstPlayer::iteration(const Arguments& args)
{
    HandleScope scope;

    GstPlayer* obj = ObjectWrap::Unwrap<GstPlayer>(args.This());

    player_iteration(obj->player);

    return scope.Close(Undefined());
}

Handle<Value> GstPlayer::stop(const Arguments& args)
{
    HandleScope scope;

    g_print("Stopping player\n");

    GstPlayer* obj = ObjectWrap::Unwrap<GstPlayer>(args.This());

    player_stop(obj->player);

    return scope.Close(Undefined());
}

typedef struct {
    Persistent<Function> callback;
} UserData;

void duration_query_callback(Song song, gint64 duration, UserData *user_data)
{
    Local<Object> obj = Object::New();

    obj->Set(String::NewSymbol("artist"), String::NewSymbol(song.artist));
    obj->Set(String::NewSymbol("album"), String::NewSymbol(song.album));
    obj->Set(String::NewSymbol("name"), String::NewSymbol(song.name));

    const unsigned argc = 2;

    Local<Value> argv[argc] = {obj, Number::New(duration)};

    user_data->callback->Call(Context::GetCurrent()->Global(), argc, argv);
    user_data->callback.Dispose();

    g_free((gpointer) song.artist);
    g_free((gpointer) song.album);
    g_free((gpointer) song.name);
    g_free(user_data);
}

Handle<Value> GstPlayer::query_duration(const Arguments& args)
{
    HandleScope scope;

    if (args.Length() != 2) {
        ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
        return scope.Close(Undefined());
    }

    if (!args[0]->IsObject()) {
        ThrowException(Exception::TypeError(String::New("Invalid argument, song should be an object")));
        return scope.Close(Undefined());
    }
    if (!args[1]->IsFunction()) {
        ThrowException(Exception::TypeError(String::New("Invalid argument, callback should be a functinon")));
        return scope.Close(Undefined());
    }

    Local<Object> obj = args[0]->ToObject();

    Local<String> artist = obj->Get(String::NewSymbol("artist"))->ToString();
    Local<String> album = obj->Get(String::NewSymbol("album"))->ToString();
    Local<String> name = obj->Get(String::NewSymbol("name"))->ToString();

    Song song;

    song.artist = (gchar *) g_malloc0(20),
    song.album = (gchar *) g_malloc0(20),
    song.name = (gchar *) g_malloc0(20),

    artist->WriteAscii(song.artist);
    album->WriteAscii(song.album);
    name->WriteAscii(song.name);

    UserData *user_data;

    user_data = g_new0(UserData, 1);
    user_data->callback = Persistent<Function>::New(Local<Function>::Cast(args[1]));

    song_query_duration(song, (SongDurationQueryCallback) duration_query_callback, user_data);

    return scope.Close(Undefined());
}

void GstPlayer::Initialize(Handle<Object> exports) {
    // Prepare constructor template
    Persistent<FunctionTemplate> t = Persistent<FunctionTemplate>::New(FunctionTemplate::New(new_instance));
    t->InstanceTemplate()->SetInternalFieldCount(2);
    t->SetClassName(String::NewSymbol("GstPlayer"));

    t->PrototypeTemplate()->Set(String::NewSymbol("init"),
        FunctionTemplate::New(init)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("start"),
        FunctionTemplate::New(start)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("iteration"),
        FunctionTemplate::New(iteration)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("stop"),
        FunctionTemplate::New(stop)->GetFunction());
    t->PrototypeTemplate()->Set(String::NewSymbol("queryDuration"),
        FunctionTemplate::New(query_duration)->GetFunction());

    exports->Set(String::NewSymbol("GstPlayer"), t->GetFunction());
}
