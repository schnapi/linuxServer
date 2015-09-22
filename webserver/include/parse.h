#ifndef PARSE_H
#define PARSE_H

#include "utilityHTTP.h"
#include "logger.h"

void writeResponse(int socket, Client *client);
int validateURL(int sock,Client *client);
int parseMessageSendResponse(int sock, Client *client);

#endif
