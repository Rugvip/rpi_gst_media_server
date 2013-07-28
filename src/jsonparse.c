
#include "jsonparse.h"
#include "jsonio.h"

#include "pipeline.h"

#include <json-glib/json-glib.h>
#include <string.h>


Request *read_info_request(JsonObject *rootObject)
{
    while (0) { (void) rootObject; }
    return NULL;
}

Request *read_play_request(JsonObject *rootObject)
{
    while (0) { (void) rootObject; }
    return NULL;
}

Request *read_pause_request(JsonObject *rootObject)
{
    while (0) { (void) rootObject; }
    return NULL;
}

Request *read_next_request(JsonObject *rootObject)
{
    while (0) { (void) rootObject; }
    return NULL;
}

Request *read_seek_request(JsonObject *rootObject)
{
    while (0) { (void) rootObject; }
    return NULL;
}

Request *read_volume_request(JsonObject *rootObject)
{
    while (0) { (void) rootObject; }
    return NULL;
}

Request *read_eq_request(JsonObject *rootObject)
{
    while (0) { (void) rootObject; }
    return NULL;
}


Request *jsonparse_read_request(Client *client)
{
    GError *error = NULL;
    JsonParser *parser;
    gboolean success;
    Request *request = NULL;

    parser = json_parser_new();
    success = json_parser_load_from_data(parser,client->buffer,client->buffer_len, &error);

    if (success) {
        JsonNode *root = json_parser_get_root(parser);

        if (!root || JSON_NODE_TYPE(root) != JSON_NODE_OBJECT) {
            g_warning("Invalid json received: root is not an object\n");
        }

        JsonObject *rootObject = json_node_get_object(root);

        JsonNode *typeNode = json_object_get_member(rootObject, "type");
        if (!typeNode) {
            g_warning("Invalid json received: no type defined\n");
        }
        if (!JSON_NODE_HOLDS_VALUE(typeNode)) {
            g_warning("Invalid json received: type is not a value\n");
        }
        if (json_node_get_value_type(typeNode) != G_TYPE_STRING) {
            g_warning("Invalid json received: type is not a string\n");
        }

        const gchar *str = json_node_get_string(typeNode);

        if (!strncmp(str, "info", 5)) {
            request = read_info_request(rootObject);
        } else if (!strncmp(str, "play", 6)) {
            request = read_play_request(rootObject);
        } else if (!strncmp(str, "pause", 6)) {
            request = read_pause_request(rootObject);
        } else if (!strncmp(str, "next", 5)) {
            request = read_next_request(rootObject);
        } else if (!strncmp(str, "seek", 5)) {
            request = read_seek_request(rootObject);
        } else if (!strncmp(str, "volume", 5)) {
            request = read_volume_request(rootObject);
        } else if (!strncmp(str, "eq", 5)) {
            request = read_eq_request(rootObject);
        }
    } else {
        if (error) {
            g_warning("Json parser error: %s\n", error->message);
        } else {
            g_warning("Json parser failed without error\n");
        }
    }

    g_object_unref(G_OBJECT(parser));

    return request;
}
