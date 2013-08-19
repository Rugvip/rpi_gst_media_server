#include <node.h>
#include <v8.h>

#include "../src/common.h"
#include "../src/player.h"

#include <glib.h>

using namespace v8;

Handle<Value> say_wat(const Arguments& args)
{
    HandleScope scope;

    g_print("Hello");

    Player *player = player_alloc();
    player_init(player);
    player_start(player);

    return scope.Close(String::New(" World"));
}

void init(Handle<Object> exports)
{
    exports->Set(String::NewSymbol("derp"), FunctionTemplate::New(say_wat)->GetFunction());
}

NODE_MODULE(test, init)
