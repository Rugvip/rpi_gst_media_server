#include <node.h>
#include <v8.h>

#include "gst_player.h"

#include "../src/common.h"
#include "../src/player.h"

#include <glib.h>

using namespace v8;

Handle<Value> Initialize_gst(const Arguments& args)
{

    HandleScope scope;

    if (args.Length() > 1) {
        ThrowException(Exception::TypeError(String::New("Too many arguments")));
        return scope.Close(Undefined());
    }

    g_setenv("GST_DEBUG_DUMP_DOT_DIR", "/home/rugvip/dev", TRUE);

    if (args.Length() == 0) {
        int argc = 2;
        char *argv[] = {
            "gmp3",
            "--gst-debug-level=1"
        };

        gst_init(&argc, (char ***) &argv);
    } else {
        if (!args[0]->IsArray()) {
            ThrowException(Exception::TypeError(String::New("Invalid argument, argv should be an array")));
            return scope.Close(Undefined());
        }

        Local<Array> array = Array::Cast(*args[0]);

        int argc = array->Length() + 1;

        char **argv = (char **) g_malloc0(argc * sizeof(char *));

        argv[0] = g_strdup("gmp3");

        for (uint32_t i = 1; i < argc; ++i) {
            Local<Value> member = array->Get(i - 1);

            if (!member->IsString()) {
                ThrowException(Exception::TypeError(String::New("Invalid argument, array member is not a string")));

                for (uint32_t j = 0; j < i; ++j) {
                    g_free(argv[j]);
                }
                g_free(argv);

                return scope.Close(Undefined());
            }

            Local<String> str = member->ToString();

            argv[i] = (char *) g_malloc0(str->Length() + 1);

            str->WriteAscii(argv[i]);
        }

        gst_init(&argc, &argv);

        for (uint32_t i = 0; i < argc; ++i) {
            g_free(argv[i]);
        }
        g_free(argv);
    }

    return scope.Close(Undefined());
}

void Initialize(Handle<Object> exports)
{
    GstPlayer::Initialize(exports);
    exports->Set(String::NewSymbol("init"),
        FunctionTemplate::New(Initialize_gst)->GetFunction());
}

NODE_MODULE(net_player, Initialize)
