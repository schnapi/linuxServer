#ifndef PARSE_H
#define PARSE_H

#include "utilityHTTP.h"
#include "logger.h"

void writeResponse(int socket, Client *client);
int validateURL(int sock,Client *client);
void parseMessageSendResponse(int socket, Client *client, int readSize);

#endif
