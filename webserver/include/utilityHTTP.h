#ifndef UTILITYHTTP_H
#define UTILITYHTTP_H

#include <stdio.h>
#include <stdlib.h>
#include<string.h>    //strlen
#include <limits.h> //PATH_MAX
#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include "dateTime.h"

#define BUFFERSIZE 4096
#define SERVERNAME "RoSa/1.0" //RoSa is the name of web server - acronym for Romain, Sandi
#define SIZEOFDATETIMEGMT 40
#define SIZEOFDATETIMECLF 40
#define SIZEOFDATETIMELOG 40

typedef struct {
	char *extension;
	char *filetype;
} FileExtensions;

typedef struct {
	int port;
	int isDaemon;
	char* configurationFile;
	char* rootDirectory;
	char* handlingMethod;
	char* customLog;
	char* nodeName;
	char *executionDirectory;
	char *statusCodesDir;
	FileExtensions fileSupport[14]; 
} ServerConfigurations;

typedef struct {
	long contentLength;
	short simple;
	short closeConnection;
	char buffer[BUFFERSIZE]; //4kB best size for sending data
	char* statusCode;
	char* filePath;
	char* fileType;
	char lastModified[SIZEOFDATETIMEGMT];
	char IPAddress[INET_ADDRSTRLEN];
	
} HTTPResponse;
typedef struct {
	char message[10000];
	char* method;
	char uri[PATH_MAX];
	char httpVersion[8];
} HTTPRequest;

typedef struct {
	HTTPResponse httpRes;
	HTTPRequest httpReq;
} Client;

//global variables---------------------------------------------------
#ifdef SERVER

	ServerConfigurations sc={
		.fileSupport= { 
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
			{"css", "text/css" },  
			{"js", "application/javascript" },  
			{"ogg", "audio/ogg" },  
			{0,0} }
	};
	char dateTimeGMT[SIZEOFDATETIMEGMT];
	char dateTimeCLF[SIZEOFDATETIMECLF];
	char dateTimeLog[SIZEOFDATETIMELOG];
#else
	//global variables created in SERVER file
	//this is a linker to global variables
	//note - static variables cannot be shared between files
	extern char dateTimeGMT[SIZEOFDATETIMEGMT];
	extern char dateTimeCLF[SIZEOFDATETIMECLF];
	extern char dateTimeLog[SIZEOFDATETIMELOG];
	extern FileExtensions fileSupport;
	extern ServerConfigurations sc;
#endif

// functions---------------------------------------------------------
void generateHeader(HTTPResponse *httpRes);
void UtilityFree(HTTPResponse *httpRes);


#endif
