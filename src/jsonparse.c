
#include "jsonparse.h"

#include <math.h>
#include <string.h>

#define INPUT(obj) ((Input *) (obj))

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

static Input *read_input_info(JsonObject *object)
{
    InputInfo *input;

    UNUSED(object);

    input = g_new0(InputInfo, 1);

    input->input.type = INPUT_INFO;

    return INPUT(input);
}

static Input *read_input_play(JsonObject *object)
{
    InputPlay *input;
    const gchar *artist, *album, *song;

    input = g_new0(InputPlay, 1);

    input->input.type = INPUT_PLAY;

    artist = get_string(object, "artist");
    album = get_string(object, "album");
    song = get_string(object, "song");

    if (artist && album && song) {
        input->song.artist = artist;
        input->song.album = album;
        input->song.name = song;

        if (get_int(object, "time", &input->time)) {
            return INPUT(input);
        }
    }
    g_free(input);
    return NULL;
}

static Input *read_input_pause(JsonObject *object)
{
    InputPause *input;

    input = g_new0(InputPause, 1);

    input->input.type = INPUT_PAUSE;

    if (get_int(object, "time", &input->time)) {
        return INPUT(input);
    }
    g_free(input);
    return NULL;
}

static Input *read_input_next(JsonObject *object)
{
    InputNext *input;
    const gchar *artist, *album, *song;

    input = g_new0(InputNext, 1);

    input->input.type = INPUT_NEXT;

    artist = get_string(object, "artist");
    album = get_string(object, "album");
    song = get_string(object, "song");

    if (artist && album && song) {
        input->song.artist = artist;
        input->song.album = album;
        input->song.name = song;

        if (get_int(object, "time", &input->time)) {
            return INPUT(input);
        }
    }
    g_free(input);
    return NULL;
}

static Input *read_input_seek(JsonObject *object)
{
    InputSeek *input;

    input = g_new0(InputSeek, 1);

    input->input.type = INPUT_SEEK;

    if (get_int(object, "time", &input->time)) {
        return INPUT(input);
    }

    g_free(input);
    return NULL;
}

static Input *read_input_volume(JsonObject *object)
{
    InputVolume *input;

    input = g_new0(InputVolume, 1);

    input->input.type = INPUT_VOLUME;

    if (get_double(object, "volume", &input->volume)) {
        return INPUT(input);
    }

    g_free(input);
    return NULL;
}

static void read_eq_foreach(JsonArray *array, guint i, JsonNode *node, InputEq *input)
{
    UNUSED(array);

    if (!JSON_NODE_HOLDS_VALUE(node)) {
        input->bands[i] = NAN;
    }

    if (json_node_get_value_type(node) != G_TYPE_DOUBLE) {
        input->bands[i] = NAN;
    }

    input->bands[i] = json_node_get_double(node);
}

static Input *read_input_eq(JsonObject *object)
{
    InputEq *input;
    JsonArray *array;
    gint i;

    input = g_new0(InputEq, 1);

    input->input.type = INPUT_EQ;

    array = get_array(object, "bands");

    if (!array || json_array_get_length(array) != NUM_EQ_BANDS) {
        g_free(input);
        return NULL;
    }

    json_array_foreach_element(array, (JsonArrayForeach) read_eq_foreach, input);

    for (i = 0; i < NUM_EQ_BANDS; i++) {
        if (isnan(input->bands[i])) {
            g_free(input);
            return NULL;
        }
    }

    return INPUT(input);
}

static JsonObject *safe_read_input(JsonNode *root)
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

static const gchar *safe_read_type(JsonObject *object)
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

Input *jsonparse_read_input(Player *player, JsonParser *parser)
{
    GError *error = NULL;
    gboolean success;
    Input *input = NULL;

    success = json_parser_load_from_data(parser, player->buffer, player->buffer_len, &error);

    if (success) {
        JsonObject *rootObject;
        const gchar *str;

        rootObject = safe_read_input(json_parser_get_root(parser));
        str = safe_read_type(rootObject);

        if (str) {
            if (!strncmp(str, "info", 5)) {
                input = read_input_info(rootObject);
            } else if (!strncmp(str, "play", 6)) {
                input = read_input_play(rootObject);
            } else if (!strncmp(str, "pause", 6)) {
                input = read_input_pause(rootObject);
            } else if (!strncmp(str, "next", 5)) {
                input = read_input_next(rootObject);
            } else if (!strncmp(str, "seek", 5)) {
                input = read_input_seek(rootObject);
            } else if (!strncmp(str, "volume", 5)) {
                input = read_input_volume(rootObject);
            } else if (!strncmp(str, "eq", 5)) {
                input = read_input_eq(rootObject);
            }
        }

        if (input == NULL) {
            g_warning("Malformed packet: %s\n", player->buffer);
        } else {
            input->player = player;
        }
    } else {
        if (error) {
            g_warning("Json parser error: %s\n", error->message);
        } else {
            g_warning("Json parser failed without error\n");
        }
    }

    return input;
}
