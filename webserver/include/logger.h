#ifndef LOGGER_H
#define LOGGER_H

#include<stdio.h>
#include <stdlib.h>
#include<string.h>    //strlen
#include<arpa/inet.h> //inet_addr
#include <errno.h>
#include <unistd.h> //read and write and close
#include <syslog.h>
#include <fcntl.h> // for open
#include "utilityHTTP.h"
#include "parse.h"

#define ERROR -1
#define LOG 0
#define BADREQUEST 400
#define FORBIDDEN 403
#define NOTFOUND 404
#define INTERNALSERVERERROR 500
#define NOTIMPLEMENTED 501

void writeToLogFile(char* filePath, char *logMessage,int error);
int checkErrno(int socket, Client *client);
void loggerClient(int socket,int method,Client *client, char *s1, char *s2);
void loggerServer(int level,char *s1,char *s2,char* clientIp);
void loggerSuccess(char* method, Client *client);
#endif
