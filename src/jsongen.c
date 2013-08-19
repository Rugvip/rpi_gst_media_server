#include "jsongen.h"

#include <glib.h>
#include <json-glib/json-glib.h>

#define add_string(builder, name, value) json_builder_add_string_value \
    (json_builder_set_member_name(builder, name), value)
#define add_int(builder, name, value) json_builder_add_int_value \
    (json_builder_set_member_name(builder, name), value)
#define add_double(builder, name, value) json_builder_add_double_value \
    (json_builder_set_member_name(builder, name), value)

static void write_packet(Output *output, JsonBuilder *builder)
{
    GError *error = NULL;
    JsonGenerator *generator;

    generator = json_generator_new();
    json_generator_set_root(generator, json_builder_get_root(builder));
    json_generator_to_stream(generator, output->player->out, NULL, &error);

    if (error) {
        g_warning("Error writing json output stream: %s\n", error->message);
        g_clear_error(&error);
    }

    g_object_unref(G_OBJECT(generator));
    g_object_unref(G_OBJECT(builder));
}

void jsongen_playing(OutputPlaying *output)
{
    JsonBuilder *builder = json_builder_new();

    json_builder_begin_object(builder);

    add_string(builder, "type", "played");
    add_string(builder, "artist", output->song.artist);
    add_string(builder, "album", output->song.album);
    add_string(builder, "song", output->song.name);
    add_int(builder, "duration", output->duration);
    add_int(builder, "position", output->position);

    json_builder_end_object(builder);

    write_packet(&output->output, builder);
}

void jsongen_paused(OutputPaused *output)
{
    JsonBuilder *builder = json_builder_new();

    json_builder_begin_object(builder);

    add_string(builder, "type", "paused");
    write_packet(&output->output, builder);

    json_builder_end_object(builder);

    write_packet(&output->output, builder);
}

void jsongen_eq(OutputEq *output)
{
    int i;
    JsonBuilder *builder = json_builder_new();

    json_builder_begin_object(builder);

    add_string(builder, "type", "eq");

    json_builder_begin_array(json_builder_set_member_name(builder, "bands"));
    for (i = 0; i < NUM_EQ_BANDS; ++i) {
        json_builder_add_double_value(builder, output->bands[i]);
    }
    json_builder_end_array(builder);

    json_builder_end_object(builder);

    write_packet(&output->output, builder);
}

void jsongen_volume(OutputVolume *output)
{
    JsonBuilder *builder = json_builder_new();

    json_builder_begin_object(builder);

    add_string(builder, "type", "volume");
    add_double(builder, "volume", output->volume);

    json_builder_end_object(builder);

    write_packet(&output->output, builder);
}

void jsongen_info(OutputInfo *output)
{
    JsonBuilder *builder = json_builder_new();

    json_builder_begin_object(builder);

    add_string(builder, "type", "info");
    add_string(builder, "artist", output->song.artist);
    add_string(builder, "album", output->song.album);
    add_string(builder, "song", output->song.name);
    add_int(builder, "duration", output->duration);
    add_int(builder, "position", output->position);

    json_builder_end_object(builder);

    write_packet(&output->output, builder);
}

void jsongen_duration_result(OutputDurationResult *output)
{
    JsonBuilder *builder = json_builder_new();

    json_builder_begin_object(builder);

    add_string(builder, "type", "duration_result");
    add_string(builder, "artist", output->song.artist);
    add_string(builder, "album", output->song.album);
    add_string(builder, "song", output->song.name);
    add_int(builder, "duration", output->duration);

    json_builder_end_object(builder);

    write_packet(&output->output, builder);
}
