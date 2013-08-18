#include "jsonio.h"
#include "jsongen.h"
#include "jsonparse.h"

void jsonio_read(Player *player)
{
    Input *input;
    JsonParser *parser;

    parser = json_parser_new();
    input = jsonparse_read_input(player, parser);

    if (!input) {
        g_object_unref(G_OBJECT(parser));
        return;
    }

    player->handlers[input->type](input);

    g_object_unref(G_OBJECT(parser));
    g_free(input);
}

void jsonio_set_input_handler(Player *player, InputType type, InputHandler handler)
{
    player->handlers[type] = handler;
}

void jsonio_write(gpointer _output)
{
    Output *output;

    output = (Output *)(_output);

    switch(output->type) {
    case OUTPUT_PLAYING:
        jsongen_playing((OutputPlaying *)(output));
        break;
    case OUTPUT_PAUSED:
        jsongen_paused((OutputPaused *)(output));
        break;
    case OUTPUT_EQ:
        jsongen_eq((OutputEq *)(output));
        break;
    case OUTPUT_VOLUME:
        jsongen_volume((OutputVolume *)(output));
        break;
    case OUTPUT_INFO:
        jsongen_info((OutputInfo *)(output));
        break;
    default:
        g_warning("Invalid output type\n");
    }
}
