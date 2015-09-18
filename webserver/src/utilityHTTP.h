#ifndef UTILITYHTTP_H
#define UTILITYHTTP_H

#include <stdio.h>
#include <stdlib.h>
#include<string.h>    //strlen
#include <limits.h> //PATH_MAX
#include "dateTime.h"

#define BUFFERSIZE 4096
#define SERVERNAME "RoSa/1.0" //RoSa is the name of web server - acronym for Romain, Sandi
#define SIZEOFDATETIMEGMT 40

typedef struct {
	char *extension;
	char *filetype;
} FileExtensions;

typedef struct {
	int http_status,keep_alive;
	long contentLength;
	short simple;
	char buffer[BUFFERSIZE]; //4kB best size for sending data
	char* statusCode;
	char* filePath;
	char* fileType;
	char* method;
	char lastModified[SIZEOFDATETIMEGMT];
	
} HTTPResponse;
typedef struct {
	char message[10000];
	char uri[PATH_MAX];
	char httpVersion[8];
	char host[10000];
} HTTPRequest;
#ifdef SERVER
char dateTimeGMT[SIZEOFDATETIMEGMT];
FileExtensions fileSupport [] = { 
	{"html","text/html" }, 
	{"htm", "text/html" },  
	{"gif", "image/gif" },  
	{"jpg", "image/jpg" }, 
	{"jpeg","image/jpeg"},
	{"png", "image/png" },  
	{"ico", "image/ico" },  
	{"zip", "image/zip" },  
	{"gz",  "image/gz"  },  
	{"tar", "image/tar" },  
	{0,0} };
#else
//global variables created in SERVER file
//this is a linker to global variables
//note - static variables cannot be shared between files
extern char dateTimeGMT[SIZEOFDATETIMEGMT];
extern FileExtensions fileSupport;
#endif

void generateHeader(HTTPResponse *httpRes);
void UtilityFree(HTTPResponse *httpRes);

#endif
