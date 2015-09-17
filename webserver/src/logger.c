#include "logger.h"

int checkErrno(int socket, char* filePath){
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
			logger(socket,FORBIDDEN, "Read or search permission was denied for a component of the path prefix.",filePath);
		break;
		case EINVAL:
			logger(socket,NOTFOUND, "File not found - realpath",filePath);
		break;
		case EIO:
			logger(socket,BADREQUEST, "An I/O error occurred while reading from the filesystem.",filePath);
		break;
		case ELOOP:
			logger(socket,BADREQUEST, "Too many symbolic links were encountered in translating the pathname.",filePath);
		break;
		case ENAMETOOLONG:
			logger(socket,BADREQUEST, "A component of a pathname exceeded NAME_MAX characters, or an entire pathname exceeded PATH_MAX characters.",filePath);
		break;
		case ENOMEM:
			logger(socket,BADREQUEST, "Out of memory.",filePath);
		break;	
		case ENOENT:
			logger(socket,NOTFOUND, "The named file does not exist.",filePath);
		break;	
		case ENOTDIR:
			logger(socket,BADREQUEST, "A component of the path prefix is not a directory.",filePath);
		break;	
	}
	return errno;
}


void logger(int socket,int method, char *s1, char *s2)
{
	
	printf("test2: %d\n",method);
	FILE * fp; // file pointer
	char *logMessage;
	switch (method) {
	case ERROR:
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
		write(socket, "HTTP/1.1 500 Internal Server Error\nContent-Length: 245\nConnection: close\nContent-Type: text/html\n\n<html><head>\n<title>500 Internal Server Error</title>\n</head><head>\n<title>500 Internal Server Error</title>\n</head>\n<body>\n<h1>500 Internal Server Error</h1>\n<p>ERROR WITH AN CONFIG FILE,</p>\n<p>or something else went wrong.</p>\n</body></html>\n",343);
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


		
