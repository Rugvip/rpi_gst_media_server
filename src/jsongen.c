#include "jsongen.h"

#include "pipeline.h"

#include <json-glib/json-glib.h>

static void write_json_response(GOutputStream *stream, JsonNode *root)
{
    GError *error = NULL;
    JsonGenerator *generator;

    generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_to_stream(generator, stream, NULL, &error);

    if (error) {
        g_warning("Error sending json stream: %s\n", error->message);
    }
    g_clear_error(&error);
    g_object_unref(G_OBJECT(generator));
}

static JsonBuilder *build_object()
{
    JsonBuilder *builder;

    builder = json_builder_new();
    json_builder_begin_object(builder);

    return builder;
}

static void end_build_object(JsonBuilder *builder, GOutputStream *stream)
{
    json_builder_end_object(builder);
    write_json_response(stream, json_builder_get_root(builder));
    g_object_unref(G_OBJECT(builder));
}

void jsongen_playback_info(Client *client, JSON_PlaybackInfo *info)
{
    JsonBuilder *builder = build_object();

    json_builder_add_string_value(json_builder_set_member_name(builder,
        "category"), "playback");
    json_builder_add_string_value(json_builder_set_member_name(builder,
        "action"), "info");
    json_builder_add_int_value(json_builder_set_member_name(builder,
        "duration"), info->duration);
    json_builder_add_int_value(json_builder_set_member_name(builder,
        "position"), info->position);

    end_build_object(builder, client->out);
}

void jsongen_playback_start(Client *client, JSON_PlaybackInfo *info)
{
    JsonBuilder *builder = build_object();

    json_builder_add_string_value(json_builder_set_member_name(builder,
        "category"), "playback");
    json_builder_add_string_value(json_builder_set_member_name(builder,
        "action"), "start");
    json_builder_add_int_value(json_builder_set_member_name(builder,
        "duration"), info->duration);
    json_builder_add_int_value(json_builder_set_member_name(builder,
        "position"), info->position);

    end_build_object(builder, client->out);
}
