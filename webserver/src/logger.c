#include "logger.h"

int checkErrno(int socket, HTTPResponse *httpRes){
		printf("test1: %d\n",EACCES);
		printf("test2: %d\n",EINVAL);
		printf("test3: %d\n",EIO);
		printf("test4: %d\n",ELOOP);
		printf("test5: %d\n",ENAMETOOLONG);
		printf("test6: %d\n",ENOMEM);
		printf("test7: %d\n",ENOENT);
		printf("test8: %d\n",ENOTDIR);
	switch(errno) {
		case EACCES:
			logger(socket,FORBIDDEN,httpRes, "Read or search permission was denied for a component of the path prefix.",httpRes->filePath);
		break;
		case EINVAL:
			logger(socket,NOTFOUND,httpRes, "File not found - realpath",httpRes->filePath);
		break;
		case EIO:
			logger(socket,INTERNALSERVERERROR,httpRes, "An I/O error occurred while reading from the filesystem.",httpRes->filePath);
		break;
		case ELOOP:
			logger(socket,BADREQUEST,httpRes, "Too many symbolic links were encountered in translating the pathname.",httpRes->filePath);
		break;
		case ENAMETOOLONG:
			logger(socket,BADREQUEST,httpRes, "A component of a pathname exceeded NAME_MAX characters, or an entire pathname exceeded PATH_MAX characters.",httpRes->filePath);
		break;
		case ENOMEM:
			logger(socket,INTERNALSERVERERROR,httpRes, "Out of memory.",httpRes->filePath);
		break;	
		case ENOENT:
			logger(socket,NOTFOUND,httpRes, "The named file does not exist.",httpRes->filePath);
		break;	
		case ENOTDIR:
			logger(socket,BADREQUEST,httpRes, "A component of the path prefix is not a directory.",httpRes->filePath);
		break;	
	}
	return errno;
}


void logger(int socket,int method, HTTPResponse *httpRes, char *s1, char *s2)
{
	FILE * fp; // file pointer
	char *logMessage,*content;
	switch (method) {
	case ERROR:
		asprintf(&logMessage,"ERROR: %s:%s Errno=%d exiting pid=%d\n",s1, s2, errno,getpid()); 
		break;
	case LOG:
		asprintf(&logMessage,"INFO: %s:%s:%d\n",s1, s2,socket);
		break;
	case BADREQUEST:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>403 Bad Request</title>\n</head><body>\n<h1>Bad Request</h1>\n<p>The server cannot or will not process the request due to something that is perceived to be a client error (e.g., malformed request syntax, invalid request message framing, or deceptive request routing)</p>\n</body></html>";
		httpRes->statusCode="400 Bad Request";
		httpRes->contentLength = strlen(content);
		asprintf(&logMessage,"Bad Request: %s:%s\n",s1, s2);
		break;
	case FORBIDDEN:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\n<p>The requested URL, file type or operation is not allowed on this simple static file webserver,</p><p>or file extension type not supported.</p>\n</body></html>";
		httpRes->statusCode="403 Forbidden";
		httpRes->contentLength = strlen(content);
		asprintf(&logMessage,"FORBIDDEN: %s:%s\n",s1, s2);
		break;
	case NOTFOUND:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>404 Not Found</title>\n</head>\n<body>\n<h1>Not Found</h1>\n<p>The requested URL was not found on this server.</p>\n</body></html>";
		httpRes->statusCode="404 Not Found";
		httpRes->contentLength = strlen(content);
		asprintf(&logMessage,"Not Found: %s:%s\n",s1, s2);
		break;
	case INTERNALSERVERERROR:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>500 Internal Server Error</title>\n</head>\n<body>\n<h1>500 Internal Server Error</h1>\n<p>ERROR WITH AN CONFIG FILE,</p>\n<p>or something else went wrong.</p>\n</body></html>";
		httpRes->statusCode="500 Internal Server Error";
		httpRes->contentLength = strlen(content);
		asprintf(&logMessage,"Internal Server Error: %s:%s\n",s1, s2);
		break;
	case NOTIMPLEMENTED:
		content="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">\n<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<title>501 Not Implemented</title>\n</head><body>\n<h1>Not Implemented</h1>\n<p>This type of operation is not supported on this webserver. Server support only GET and HEAD methods.</p>\n</body></html>";
		httpRes->statusCode="501 Not Implemented";
		httpRes->contentLength = strlen(content);
		asprintf(&logMessage,"Not Implemented: %s:%s\n",s1, s2);
		break;
	}
	strncpy(httpRes->lastModified , "Fri, 18 Sep 2015 15:24:38 GMT", 29);
	generateHeader(httpRes); //utilityHTTP.c
	//write header and then content
	write(socket,httpRes->buffer,strlen(httpRes->buffer));
	write(socket,content,httpRes->contentLength);
	//a-append data
	// if file not exist create it
	if((fp = fopen("log", "a")) != NULL) {
		fputs(logMessage,fp);    
		fclose(fp);
	}
	//free memory due of use asprintf function
	free(logMessage);
	//close the socket due an error occurred
	close(socket);
}


		
