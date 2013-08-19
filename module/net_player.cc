#include <node.h>
#include <v8.h>

#include "gst_player.h"

#include "../src/common.h"
#include "../src/player.h"

#include <glib.h>

using namespace v8;

void Initialize(Handle<Object> exports)
{
    GstPlayer::Initialize(exports);
}

NODE_MODULE(net_player, Initialize)
