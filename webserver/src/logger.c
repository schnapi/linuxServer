#include "../include/logger.h"

int checkErrno(int socket, Client *client){
		printf("test1: %d\n",EACCES);
		printf("test2: %d\n",EINVAL);
		printf("test3: %d\n",EIO);
		printf("test4: %d\n",ELOOP);
		printf("test5: %d\n",ENAMETOOLONG);
		printf("test6: %d\n",ENOMEM);
		printf("test7: %d\n",ENOENT);
		printf("test8: %d\n",ENOTDIR);
	printf("Errno is: %d\n",errno);
	switch(errno) {
		case EACCES:
			loggerClient(socket,FORBIDDEN,client, "Read or search permission was denied for a component of the path prefix.",client->httpRes.filePath);
		break;
		case EINVAL:
			loggerClient(socket,NOTFOUND,client, "File not found - realpath",client->httpRes.filePath);
		break;
		case EIO:
			loggerClient(socket,INTERNALSERVERERROR,client, "An I/O error occurred while reading from the filesystem.",client->httpRes.filePath);
		break;
		case ELOOP:
			loggerClient(socket,BADREQUEST,client, "Too many symbolic links were encountered in translating the pathname.",client->httpRes.filePath);
		break;
		case ENAMETOOLONG:
			loggerClient(socket,BADREQUEST,client, "A component of a pathname exceeded NAME_MAX characters, or an entire pathname exceeded PATH_MAX characters.",client->httpRes.filePath);
		break;
		case ENOMEM:
			loggerClient(socket,INTERNALSERVERERROR,client, "Out of memory.",client->httpRes.filePath);
		break;	
		case ENOENT:
			loggerClient(socket,NOTFOUND,client, "The named file does not exist.",client->httpRes.filePath);
		break;	
		case ENOTDIR:
			loggerClient(socket,BADREQUEST,client, "A component of the path prefix is not a directory.",client->httpRes.filePath);
		break;	
	}
	return errno;
}

void writeToLogFile(char* filePath, char *logMessage) {
	FILE * fp; // file pointer
	//a-append data
	// if file not exist create it
	if((fp = fopen(filePath, "a")) != NULL) {
		fputs(logMessage,fp);    
		fclose(fp);
	}
}
void loggerServer(int level,char *s1,char *s2,char* clientIp) {
	char *levelString;
	switch (level) {
	case LOG_EMERG:
		levelString="emergency"; break;
	case LOG_ALERT:
		levelString="alert"; break;
	case LOG_CRIT:
		levelString="critical"; break;
	case LOG_ERR:
		levelString="error"; break;
	case LOG_WARNING:
		levelString="warning"; break;
	case LOG_NOTICE:
		levelString="notice"; break;
	case LOG_INFO:
		levelString="info"; break;
	case LOG_DEBUG:
		levelString="debug"; break;
	}
	char *logMessage;
	getDateTimeLog(dateTimeLog, SIZEOFDATETIMELOG);
	//if server error
	if(clientIp==NULL){
		asprintf(&logMessage,"[%s] [%s] %s %s\n", dateTimeLog, levelString, s1,s2);
	} 
	//else client error
	else {
		asprintf(&logMessage,"[%s] [%s] [client %s] %s %s\n",dateTimeLog, levelString, clientIp, s1,s2);
	}
	
	//if is syslog	
	if(sc.customLog==NULL){
		syslog (level, "%s", logMessage);
	}
	else {
		writeToLogFile(sc.customLog, logMessage);
	}
	free(logMessage);
}

void loggerClient(int socket,int method, Client *client, char *s1, char *s2/*, char *logType*/)
{
	char *content;
	switch (method) {
	case BADREQUEST:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>403 Bad Request</title>\n</head><body>\n<h1>Bad Request</h1>\n<p>The server cannot or will not process the request due to something that is perceived to be a client error (e.g., malformed request syntax, invalid request message framing, or deceptive request routing)</p>\n</body></html>";
		client->httpRes.statusCode="400 Bad Request";
		client->httpRes.contentLength = strlen(content);
		break;
	case FORBIDDEN:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\n<p>The requested URL, file type or operation is not allowed on this simple static file webserver,</p><p>or file extension type not supported.</p>\n</body></html>";
		client->httpRes.statusCode="403 Forbidden";
		client->httpRes.contentLength = strlen(content);
		break;
	case NOTFOUND:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>404 Not Found</title>\n</head>\n<body>\n<h1>Not Found</h1>\n<p>The requested URL was not found on this server.</p>\n</body></html>";
		client->httpRes.statusCode="404 Not Found";
		client->httpRes.contentLength = strlen(content);
		break;
	case INTERNALSERVERERROR:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>500 Internal Server Error</title>\n</head>\n<body>\n<h1>500 Internal Server Error</h1>\n<p>ERROR WITH AN CONFIG FILE,</p>\n<p>or something else went wrong.</p>\n</body></html>";
		client->httpRes.statusCode="500 Internal Server Error";
		client->httpRes.contentLength = strlen(content);
		break;
	case NOTIMPLEMENTED:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>501 Not Implemented</title>\n</head><body>\n<h1>Not Implemented</h1>\n<p>This type of operation is not supported on this webserver. Server support only GET and HEAD methods.</p>\n</body></html>";
		client->httpRes.statusCode="501 Not Implemented";
		client->httpRes.contentLength = strlen(content);
		break;
	}
	strncpy(client->httpRes.lastModified , "Fri, 18 Sep 2015 15:24:38 GMT", 29);
	generateHeader(&client->httpRes); //utilityHTTP.c
	//write header and then content
	write(socket,client->httpRes.buffer,strlen(client->httpRes.buffer));
	write(socket,content,client->httpRes.contentLength);
	
	loggerSuccess(method, client);
	//close the socket due an error occurred
	close(socket);
}

void loggerSuccess(int method, Client *client){
	//date time int Common logInfo format for logger
	getDateTimeCLF(dateTimeCLF, SIZEOFDATETIMECLF);
	if(sc.customLog==NULL){
		syslog (LOG_INFO, "%s - - [%s] \"%s %s %s\" %d %lu\n",client->httpRes.IPAddress, dateTimeCLF, client->httpReq.method,client->httpReq.uri,client->httpReq.httpVersion,method,client->httpRes.contentLength);
	}
	else 
	{
		char *logMessage;
		//creates log message in CLF (Common logFile format)
		asprintf(&logMessage,"%s - - [%s] \"%s %s %s\" %d %lu\n",client->httpRes.IPAddress, dateTimeCLF, client->httpReq.method,client->httpReq.uri,client->httpReq.httpVersion,method,client->httpRes.contentLength);
		writeToLogFile(sc.customLog, logMessage);
		//free memory due of use asprintf function
		free(logMessage);
	}
}


		
