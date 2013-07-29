#include "jsonio.h"
#include "jsonparse.h"
#include "jsongen.h"

static RequestHandler handlers[NUM_REQUEST_TYPES] = {NULL};

void jsonio_read_request(Client *client)
{
    Request *request;

    request = jsonparse_read_request(client);

    if (!request) {
        return;
    }

    handlers[request->type](request);

    g_free(request);
}

void jsonio_set_request_handler(RequestType type, RequestHandler handler)
{
    handlers[type] = handler;
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
