#include <node.h>
#include <v8.h>

#include "../src/common.h"
#include "../src/player.h"

#include <glib.h>

using namespace v8;

Handle<Value> init

Handle<Value> start(const Arguments& args)
{
    HandleScope scope;

    g_print("Starting player");

    Player *player = player_alloc();
    player_init(player);
    player_start(player);

    return scope.Close(String::New(" World"));
}

Handle<Value> alloc_player(const Arguments& args) {
    HandleScope scope;

    Local<Object> obj = Object::New();

    args[0]->ToString();

    obj->Set(String::NewSymbol("init"), FunctionTemplate::New(alloc_player)->GetFunction());
    obj->Set(String::NewSymbol("start"), FunctionTemplate::New(alloc_player)->GetFunction());

    return scope.Close(obj);
}

void Initialize(Handle<Object> exports)
{
    exports->Set(String::NewSymbol("Player"), FunctionTemplate::New(alloc_player)->GetFunction());
}

NODE_MODULE(test, Initialize)
