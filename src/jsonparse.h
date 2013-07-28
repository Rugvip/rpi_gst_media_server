#ifndef jsonparse_h
#define jsonparse_h

#include "pipeline.h"
#include "jsonio.h"

Request *jsonparse_read_request(Client *client);

#endif /* jsonparse_h */
