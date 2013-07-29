
#include "jsonparse.h"

#include <math.h>
#include <string.h>

#define REQUEST(obj) ((Request *) (obj))

static const gchar *get_string(JsonObject *object, const gchar *name)
{
    JsonNode *node;

    node = json_object_get_member(object, name);

    if (!node) {
        return NULL;
    }
    if (!JSON_NODE_HOLDS_VALUE(node)) {
        return NULL;
    }
    if (json_node_get_value_type(node) != G_TYPE_STRING) {
        return NULL;
    }

    return json_node_get_string(node);
}

static gboolean get_int(JsonObject *object, const gchar *name, gint64 *i)
{
    JsonNode *node;

    node = json_object_get_member(object, name);

    if (!node) {
        return FALSE;
    }
    if (!JSON_NODE_HOLDS_VALUE(node)) {
        return FALSE;
    }
    if (json_node_get_value_type(node) != G_TYPE_INT64) {
        return FALSE;
    }

    *i = json_node_get_int(node);
    return TRUE;
}

static gboolean get_double(JsonObject *object, const gchar *name, gdouble *d)
{
    JsonNode *node;

    node = json_object_get_member(object, name);

    if (!node) {
        return FALSE;
    }
    if (!JSON_NODE_HOLDS_VALUE(node)) {
        return FALSE;
    }
    if (json_node_get_value_type(node) != G_TYPE_INT64) {
        return FALSE;
    }

    *d = json_node_get_double(node);
    return TRUE;
}

static JsonArray *get_array(JsonObject *object, const gchar *name)
{
    JsonNode *node;

    node = json_object_get_member(object, name);

    if (!node) {
        return NULL;
    }
    if (!JSON_NODE_HOLDS_ARRAY(node)) {
        return NULL;
    }

    return json_node_get_array(node);
}

Request *read_info_request(JsonObject *object)
{
    RequestInfo *request;

    UNUSED(object);

    request = g_new0(RequestInfo, 1);

    request->request.type = REQUEST_INFO;

    return REQUEST(request);
}

Request *read_play_request(JsonObject *object)
{
    RequestPlay *request;
    const gchar *artist, *album, *song;

    request = g_new0(RequestPlay, 1);

    request->request.type = REQUEST_PLAY;

    artist = get_string(object, "artist");
    album = get_string(object, "album");
    song = get_string(object, "song");

    if (artist && album && song) {
        request->song.artist = artist;
        request->song.album = album;
        request->song.song = song;

        if (get_int(object, "time", &request->time)) {
            return REQUEST(request);
        }
    }
    g_free(request);
    return NULL;
}

Request *read_pause_request(JsonObject *object)
{
    RequestPause *request;

    request = g_new0(RequestPause, 1);

    request->request.type = REQUEST_PAUSE;

    if (get_int(object, "time", &request->time)) {
        return REQUEST(request);
    }
    g_free(request);
    return NULL;
}

Request *read_next_request(JsonObject *object)
{
    RequestNext *request;
    const gchar *artist, *album, *song;

    request = g_new0(RequestNext, 1);

    request->request.type = REQUEST_NEXT;

    artist = get_string(object, "artist");
    album = get_string(object, "album");
    song = get_string(object, "song");

    if (artist && album && song) {
        request->song.artist = artist;
        request->song.album = album;
        request->song.song = song;

        if (get_int(object, "time", &request->time)) {
            return REQUEST(request);
        }
    }
    g_free(request);
    return NULL;
}

Request *read_seek_request(JsonObject *object)
{
    RequestSeek *request;

    request = g_new0(RequestSeek, 1);

    request->request.type = REQUEST_SEEK;

    if (get_int(object, "time", &request->time)) {
        return REQUEST(request);
    }

    g_free(request);
    return NULL;
}

Request *read_volume_request(JsonObject *object)
{
    RequestVolume *request;

    request = g_new0(RequestVolume, 1);

    request->request.type = REQUEST_VOLUME;

    if (get_double(object, "volume", &request->volume)) {
        return REQUEST(request);
    }

    g_free(request);
    return NULL;
}

void read_eq_foreach(JsonArray *array, guint i, JsonNode *node, RequestEq *request)
{
    UNUSED(array);

    if (!JSON_NODE_HOLDS_VALUE(node)) {
        request->bands[i] = NAN;
    }

    if (json_node_get_value_type(node) != G_TYPE_DOUBLE) {
        request->bands[i] = NAN;
    }

    request->bands[i] = json_node_get_double(node);
}

Request *read_eq_request(JsonObject *object)
{
    RequestEq *request;
    JsonArray *array;
    gint i;

    request = g_new0(RequestEq, 1);

    request->request.type = REQUEST_EQ;

    array = get_array(object, "bands");

    if (!array || json_array_get_length(array) != NUM_EQ_BANDS) {
        return NULL;
    }

    json_array_foreach_element(array, (JsonArrayForeach) read_eq_foreach, request);

    for (i = 0; i < NUM_EQ_BANDS; i++) {
        if (isnan(request->bands[i])) {
            g_free(request);
            return NULL;
        }
    }

    return REQUEST(request);
}

JsonObject *safe_read_packet(JsonNode *root)
{
    if (!root) {
        g_warning("Invalid json received: root is null\n");
        return NULL;
    }

    if (JSON_NODE_TYPE(root) != JSON_NODE_OBJECT) {
        g_warning("Invalid json received: root is not an object\n");
        return NULL;
    }

    return json_node_get_object(root);
}

const gchar *safe_read_type(JsonObject *object)
{
    if (!object) {
        return NULL;
    }

    JsonNode *typeNode;

    typeNode = json_object_get_member(object, "type");

    if (!typeNode) {
        g_warning("Invalid json received: no type defined\n");
        return NULL;
    }

    if (!JSON_NODE_HOLDS_VALUE(typeNode)) {
        g_warning("Invalid json received: type is not a value\n");
        return NULL;
    }

    if (json_node_get_value_type(typeNode) != G_TYPE_STRING) {
        g_warning("Invalid json received: type is not a string\n");
        return NULL;
    }

    return json_node_get_string(typeNode);
}

Request *jsonparse_read_request(Client *client, JsonParser *parser)
{
    GError *error = NULL;
    gboolean success;
    Request *request = NULL;

    success = json_parser_load_from_data(parser, client->buffer, client->buffer_len, &error);

    if (success) {
        JsonObject *rootObject;
        const gchar *str;

        rootObject = safe_read_packet(json_parser_get_root(parser));
        str = safe_read_type(rootObject);

        if (str) {
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
        }

        if (request == NULL) {
            g_warning("Malformed packet: %s\n", client->buffer);
        } else {
            request->client = client;
        }
    } else {
        if (error) {
            g_warning("Json parser error: %s\n", error->message);
        } else {
            g_warning("Json parser failed without error\n");
        }
    }

    return request;
}
