#ifndef LOGGER_H
#define LOGGER_H

#include<stdio.h>
#include <stdlib.h>
#include <errno.h>

#define LOG 0
#define BADREQUEST 400
#define FORBIDDEN 403
#define NOTFOUND 404
#define NOTIMPLEMENTED 501
void logger(int socket,int method, char *s1, char *s2);

#endif
