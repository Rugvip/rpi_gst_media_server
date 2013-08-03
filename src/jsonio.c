#include "jsonio.h"
#include "jsongen.h"
#include "jsonparse.h"

JsonPacket *jsonio_response_playing_packet(ResponsePlaying *response)
{
    return jsongen_playing(response->song, response->duration, response->position);
}

JsonPacket *jsonio_response_paused_packet(ResponsePaused *response)
{
    return jsongen_paused(response->time);

}

JsonPacket *jsonio_response_eq_packet(ResponseEq *response)
{
    return jsongen_eq(response->bands);

}

JsonPacket *jsonio_response_volume_packet(ResponseVolume *response)
{
    return jsongen_volume(response->volume);

}

JsonPacket *jsonio_response_info_packet(ResponseInfo *response)
{
    return jsongen_info(response->song, response->duration, response->position);
}


void jsonio_read_request(Client *client)
{
    Request *request;
    JsonParser *parser;

    parser = json_parser_new();
    request = jsonparse_read_request(client, parser);

    if (!request) {
        g_object_unref(G_OBJECT(parser));
        return;
    }

    client->server->handlers[request->type](request);

    g_object_unref(G_OBJECT(parser));
    g_free(request);
}

void jsonio_set_request_handler(Server *server, RequestType type, RequestHandler handler)
{
    server->handlers[type] = handler;
}

void jsonio_send_packet(Client *client, JsonPacket *packet)
{
    jsongen_write_packet(client->out, packet);
    jsongen_free_packet(packet);
}

void jsonio_broadcast_packet(Server *server, JsonPacket *packet)
{
    gint i;
    GPtrArray *clients = server->clients;
    gint len = server->clients->len;

    for (i = 0; i < len; ++i) {
        jsongen_write_packet(((Client *) g_ptr_array_index(clients, i))->out, packet);
    }

    jsongen_free_packet(packet);
}
