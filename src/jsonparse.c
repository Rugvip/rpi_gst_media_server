
#include "jsonparse.h"

#include "pipeline.h"

#include <json-glib/json-glib.h>

void jsonparse_read_request(Client *client)
{
    GError *error = NULL;
    JsonParser *parser;
    gboolean success;

    parser = json_parser_new();
    success = json_parser_load_from_data(parser,client->buffer,client->buffer_len, &error);

    if (success) {
        handle_json_request(client, json_parser_get_root(parser));
    } else {
        if (error) {
            g_warning("Json parser error: %s\n", error->message);
        } else {
            g_warning("Json parser failed without error\n");
        }
    }

    g_object_unref(G_OBJECT(parser));
}
