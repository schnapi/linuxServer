#ifndef LOGGER_H
#define LOGGER_H

#include<stdio.h>
#include <stdlib.h>
#include<string.h>    //strlen
#include <errno.h>
#include <unistd.h> //read and write
#include "utilityHTTP.h"

#define ERROR -1
#define LOG 0
#define BADREQUEST 400
#define FORBIDDEN 403
#define NOTFOUND 404
#define INTERNALSERVERERROR 500
#define NOTIMPLEMENTED 501

int checkErrno(int socket, HTTPResponse *httpRes);
void logger(int socket,int method,HTTPResponse *httpRes, char *s1, char *s2);

#endif
