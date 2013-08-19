#define BUILDING_NODE_EXTENSION
#include <node.h>

#include "gst_player.h"

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

void GstPlayer::Initialize(Handle<Object> exports) {
    // Prepare constructor template
    Local<FunctionTemplate> t = FunctionTemplate::New(new_instance);
    t->SetClassName(String::NewSymbol("GstPlayer"));

    Local<Template> proto_t = t->PrototypeTemplate();
    proto_t->Set(String::NewSymbol("init"), FunctionTemplate::New(init)->GetFunction());
    proto_t->Set(String::NewSymbol("start"), FunctionTemplate::New(start)->GetFunction());

    Local<ObjectTemplate> instance_t = t->InstanceTemplate();
    instance_t->Set(String::NewSymbol("wat"), Number::New(1337));
    instance_t->SetInternalFieldCount(1);

    exports->Set(String::NewSymbol("GstPlayer"), Persistent<Function>::New(t->GetFunction()));
}
