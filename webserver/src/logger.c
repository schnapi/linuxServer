#include "logger.h"

void logger(int socket,int method, char *s1, char *s2)
{
	FILE * fp; // file pointer
	char *logMessage;
	switch (method) {
	case -1:
		asprintf(&logMessage,"ERROR: %s:%s Errno=%d exiting pid=%d\n",s1, s2, errno,getpid()); 
		break;
	case LOG:
		asprintf(&logMessage,"INFO: %s:%s:%d\n",s1, s2,socket);
		break;
	case BADREQUEST: 
		write(socket, "HTTP/1.1 400 Bad Request\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n",271);
		asprintf(&logMessage,"Bad Request: %s:%s\n",s1, s2);
		break;
	case FORBIDDEN: 
		write(socket, "HTTP/1.1 403 Forbidden\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n",271);
		asprintf(&logMessage,"FORBIDDEN: %s:%s\n",s1, s2);
		break;
	case NOTFOUND: 
		write(socket, "HTTP/1.1 404 Not Found\nContent-Length: 136\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\nThe requested URL was not found on this server.\n</body></html>\n",224);
		asprintf(&logMessage,"Not Found: %s:%s\n",s1, s2);
		break;
	case 500:
		write(socket, "HTTP/1.1 500 Internal Server Error\nContent-Length: 185\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>403 Forbidden</title>\n</head><body>\n<h1>Forbidden</h1>\nThe requested URL, file type or operation is not allowed on this simple static file webserver.\n</body></html>\n",271);
		asprintf(&logMessage,"Internal Server Error: %s:%s\n",s1, s2);
		break;
	case NOTIMPLEMENTED: write(socket, "HTTP/1.1 501 Not Implemented\nContent-Length: 203\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>501 Not Implemented</title>\n</head><body>\n<h1>Not Implemented</h1>\nThis type of operation is not supported on this webserver. Server support only GET and HEAD methods.\n</body></html>\n",295);
		asprintf(&logMessage,"Not Implemented: %s:%s\n",s1, s2);
		break;
	}
	//a-append data
	// if file not exist create it
	if((fp = fopen("log", "a")) != NULL) {
		fputs(logMessage,fp);    
		fclose(fp);
	}
	//free memory
	free(logMessage);
	//close the socket due an error occurred
	close(socket);
}
