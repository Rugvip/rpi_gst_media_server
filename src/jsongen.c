#include "jsongen.h"

#include "pipeline.h"

#include <glib.h>
#include <json-glib/json-glib.h>

#define JSON_PACKET_PRIVATE(obj) ((JsonPacketPrivate *) ((obj)->priv))
#define JSON_PACKET_BUILDER(obj) (JSON_PACKET_PRIVATE(obj)->builder)

#define add_string(builder, name, value) json_builder_add_string_value( \
    json_builder_set_member_name(builder, name), value)
#define add_int(builder, name, value) json_builder_add_int_value( \
    json_builder_set_member_name(builder, name), value)

typedef struct {
    JsonBuilder *builder;
} JsonPacketPrivate;

void jsongen_write_packet(GOutputStream *out, JsonPacket *packet)
{
    JsonNode *root;
    GError *error = NULL;
    JsonGenerator *generator;

    root = json_builder_get_root(JSON_PACKET_PRIVATE(packet)->builder);

    generator = json_generator_new();
    json_generator_set_root(generator, root);
    json_generator_to_stream(generator, out, NULL, &error);

    if (error) {
        g_warning("Error writing json output stream: %s\n", error->message);
    }
    g_clear_error(&error);
    g_object_unref(G_OBJECT(generator));
}

void jsongen_free_packet(JsonPacket *packet)
{
    JsonPacketPrivate *priv = JSON_PACKET_PRIVATE(packet);

    g_object_unref(G_OBJECT(priv->builder));

    g_free(priv);
    g_free(packet);
}

static JsonPacket *json_packet_new()
{
    JsonPacket *packet;

    packet = g_new0(JsonPacket, 1);
    packet->priv = g_new0(JsonPacketPrivate, 1);

    JSON_PACKET_PRIVATE(packet)->builder = json_builder_new();

    return packet;
}

JsonPacket *jsongen_playing(Song song, gint duration, gint position)
{
    JsonPacket *packet = json_packet_new();
    JsonBuilder *builder = JSON_PACKET_BUILDER(packet);

    json_builder_begin_object(builder);

    add_string(builder, "type", "played");
    add_string(builder, "artist", song.artist);
    add_string(builder, "album", song.album);
    add_string(builder, "song", song.song);
    add_int(builder, "duration", duration);
    add_int(builder, "position", position);

    json_builder_end_object(builder);

    return packet;
}

JsonPacket *jsongen_paused(gint position)
{
    JsonPacket *packet = json_packet_new();
    JsonBuilder *builder = JSON_PACKET_BUILDER(packet);

    json_builder_begin_object(builder);

    add_string(builder, "type", "paused");
    add_int(builder, "position", position);

    json_builder_end_object(builder);

    return packet;
}

JsonPacket *jsongen_eq(gint bands[NUM_EQ_BANDS])
{
    int i;
    JsonPacket *packet = json_packet_new();
    JsonBuilder *builder = JSON_PACKET_BUILDER(packet);

    json_builder_begin_object(builder);

    add_string(builder, "type", "eq");

    json_builder_begin_array(json_builder_set_member_name(builder, "bands"));
    for (i = 0; i < NUM_EQ_BANDS; ++i) {
        json_builder_add_int_value(builder, bands[i]);
    }
    json_builder_end_array(builder);

    json_builder_end_object(builder);

    return packet;
}

JsonPacket *jsongen_volume(gint volume)
{
    JsonPacket *packet = json_packet_new();
    JsonBuilder *builder = JSON_PACKET_BUILDER(packet);

    json_builder_begin_object(builder);

    add_string(builder, "type", "volume");
    add_int(builder, "volume", volume);

    json_builder_end_object(builder);

    return packet;
}

JsonPacket *jsongen_info(Song song, gint duration, gint position)
{
    JsonPacket *packet = json_packet_new();
    JsonBuilder *builder = JSON_PACKET_BUILDER(packet);

    json_builder_begin_object(builder);

    add_string(builder, "type", "info");
    add_string(builder, "artist", song.artist);
    add_string(builder, "album", song.album);
    add_string(builder, "song", song.song);
    add_int(builder, "duration", duration);
    add_int(builder, "position", position);

    json_builder_end_object(builder);

    return packet;
}
