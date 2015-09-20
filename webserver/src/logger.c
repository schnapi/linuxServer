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
			logger(socket,FORBIDDEN,client, "Read or search permission was denied for a component of the path prefix.",client->httpRes.filePath);
		break;
		case EINVAL:
			logger(socket,NOTFOUND,client, "File not found - realpath",client->httpRes.filePath);
		break;
		case EIO:
			logger(socket,INTERNALSERVERERROR,client, "An I/O error occurred while reading from the filesystem.",client->httpRes.filePath);
		break;
		case ELOOP:
			logger(socket,BADREQUEST,client, "Too many symbolic links were encountered in translating the pathname.",client->httpRes.filePath);
		break;
		case ENAMETOOLONG:
			logger(socket,BADREQUEST,client, "A component of a pathname exceeded NAME_MAX characters, or an entire pathname exceeded PATH_MAX characters.",client->httpRes.filePath);
		break;
		case ENOMEM:
			logger(socket,INTERNALSERVERERROR,client, "Out of memory.",client->httpRes.filePath);
		break;	
		case ENOENT:
			logger(socket,NOTFOUND,client, "The named file does not exist.",client->httpRes.filePath);
		break;	
		case ENOTDIR:
			logger(socket,BADREQUEST,client, "A component of the path prefix is not a directory.",client->httpRes.filePath);
		break;	
	}
	return errno;
}


void logger(int socket,int method, Client *client, char *s1, char *s2/*, char *logType*/)
{
	FILE * fp; // file pointer
	char *logMessage,*content;
	getDateTimeCLF(dateTimeCLF, SIZEOFDATETIMECLF);
	switch (method) {
	case ERROR:
		asprintf(&logMessage,"ERROR: %s:%s Errno=%d exiting pid=%d\n",s1, s2, errno,getpid()); 
		break;
	case LOG:
		asprintf(&logMessage,"INFO: %s:%s:%d\n",s1, s2,socket);
		break;
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
/*	syslog(LOG_ERR, "open error for %s: %m", client->httpRes.filePath);*/
/*	syslog (LOG_ERR, "Host ip");*/
/*	if(!strncmp("","syslog")){*/
/*		*/
/*	}*/
/*	else */
	{
		//creates log message in CLF (Common logFile format)
		asprintf(&logMessage,"%s - - [%s] \"%s %s %s\" %d %lu\n",client->httpRes.IPAddress, dateTimeCLF, client->httpReq.method,client->httpReq.uri,client->httpReq.httpVersion,method,client->httpRes.contentLength);
		//a-append data
		// if file not exist create it
		if((fp = fopen(sc.customLog, "a")) != NULL) {
			fputs(logMessage,fp);    
			fclose(fp);
		}
	}
	//free memory due of use asprintf function
	free(logMessage);
	//close the socket due an error occurred
	close(socket);
}


		
