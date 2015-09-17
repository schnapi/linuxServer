#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
 
#include<pthread.h> //for threading , link with lpthread
#include <limits.h> //PATH_MAX
 
#include "logger.h"
#define VERSION "1.0"
#define SERVERNAME "RoSa"
#define BUFFERSIZE 4096

/*check the tasks file*/

void *connection_handler(void *);
 
int main(int argc , char *argv[])
{     
    //Create a new socket
	//Address Family - AF_INET (this is IP version 4) Type - SOCK_STREAM (this means connection oriented TCP protocol, SOCK_DGRAM is for UDP protocol) Protocol - 0 [ or IPPROTO_IP This is IP protocol]
    int mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mySocket == -1)
    {
        perror("Could not create socket");
    }
     
    struct sockaddr_in server , client;
    //sockaddr_in structure
    server.sin_family = AF_INET; //IP version 4
    server.sin_addr.s_addr = INADDR_ANY; // we can receive packets from any of the network interface card installed in the system
    server.sin_port = htons( 8888 ); // port number
    
	int enable = 1; //enable options - nonzero
	//SO_REUSEADDR - reuse of local address
	if ( setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1 )
	{
		perror("setsockopt");
	}
    //Bind a socket - listen to connections that are comming 
    if( bind(mySocket,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("bind failed");
        return 1;
    }
    puts("bind done");
     
    //http://pubs.opengroup.org/onlinepubs/9699919799/functions/listen.html
    listen(mySocket , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    int c = sizeof(struct sockaddr_in), clientSocket, *clientSocketP;
    char *message;
	printf("%d",c);
    while( (clientSocket = accept(mySocket, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        puts("Connection accepted");
         
        pthread_t sniffer_thread;
        clientSocketP = malloc(1);
        *clientSocketP = clientSocket;
         
        if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) clientSocketP) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }
     
    if (clientSocket<0)
    {
        perror("accept failed");
        return 1;
    }
     
    return 0;
}

 /*
 * This will handle connection for each client
 * */

struct {
	char *ext;
	char *filetype;
} fileSupport [] = {
	{"gif", "image/gif" },  
	{"jpg", "image/jpg" }, 
	{"jpeg","image/jpeg"},
	{"png", "image/png" },  
	{"ico", "image/ico" },  
	{"zip", "image/zip" },  
	{"gz",  "image/gz"  },  
	{"tar", "image/tar" },  
	{"htm", "text/html" },  
	{"html","text/html" },  
	{0,0} };
typedef struct {
	int http_status,keep_alive;
	char* statusCode;
	char* filePath;
	char* fileType;
	char* method;
} HTTPResponse;
typedef struct {
	char message[10000];
	char uri[PATH_MAX];
	char httpVersion[8];
	char host[10000];
	short isSimple;
} HTTPRequest;

void writeResponse(int socket, HTTPResponse httpRes){
	char buffer[BUFFERSIZE]; //4kB best size for sending data
	FILE *fp;
	//r-read data
	if((fp = fopen(httpRes.filePath, "r")) == NULL) {
		//error can't open file or file not found // already is checked with realpath 
		//permisions denied	
		logger(socket,NOTFOUND, "Permision denied",httpRes.filePath);
	}
	else {
		fseek(fp, 0L, SEEK_END); //goes to end of the file
		long contentLength=ftell(fp);
		fseek(fp,0L,SEEK_SET); //go back to where we were
		//RoSa is the name of web server - acronym for Romain, Sandi
		sprintf(buffer,"HTTP/1.1 200 OK\nServer: %s/%s\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n",SERVERNAME, VERSION, contentLength, httpRes.fileType);
		/* Header + a blank line */
		//logger(LOG,"Header",buffer,hit);
		write(socket,buffer,strlen(buffer));
		if(httpRes.method=="GET") {
			// send file in 4KB block
			while (	(contentLength = fread(buffer, 1, BUFFERSIZE,fp)) > 0 ) {
				write(socket,buffer,contentLength);
			}
		}
		fclose(fp);
	}
	
	free(httpRes.filePath);	
}

void *connection_handler(void *mySocket)
{
    //Get the socket descriptor
    int sock = *(int*)mySocket;
    int read_size;
    HTTPResponse httpRes = {
		.fileType = NULL
	};
    HTTPRequest httpReq;
    
    //Receive a message from client
    while( (read_size = recv(sock , httpReq.message , 10000 , 0)) > 0 )
    {
        //Send the message back to client
		const char* p = &httpReq.message[0];
		int i=0;
		if(memcmp(p, "GET",3)==0){
			httpRes.method="GET";
			p+=4;//+1 space				
		}
		else if(memcmp(p, "HEAD",4)==0){
			//The HEAD method is identical to GET except that the server must not return any Entity-Body in the response. The metainformation contained in the HTTP headers in response to a HEAD request should be identical to the information sent in response to a GET request. This method can be used for obtaining metainformation about the resource identified by the Request-URI without transferring the Entity-Body itself. This method is often used for testing hypertext links for validity, accessibility, and recent modification. There is no "conditional HEAD" request analogous to the conditional GET. If an If-Modified-Since header field is included with a HEAD request, it should be ignored.
			httpRes.method="HEAD";
			p+=5;//+1 space
		}
		else {
			logger(sock,NOTIMPLEMENTED,"Only simple GET and HEAD operation supported","");
			break;
		}
			
		//decode request URI ???
		for (; ; p++)
		{
			if(*p=='\r') {
				p+=2; // \r\n two characters
				break;
			}
			else if(*p==' '){
				p++;
				break;
			}
			// get uri
			httpReq.uri[i++]=*p;
		}
		// httpReq.uri == "/" not working ????
		if(strcmp(httpReq.uri,"/") == 0) {
			strncpy(httpReq.uri, "/index.html", 11);
		}
		//file extension support
		int uriLength=strlen(httpReq.uri),extensionLength;
		for(i=0;fileSupport[i].ext != 0;i++) {
			extensionLength = strlen(fileSupport[i].ext);
			//we get last characters from uri, -extensionLength
			// if they match
			if( strncmp(&httpReq.uri[uriLength-extensionLength], fileSupport[i].ext, extensionLength) == 0) {
				//fileType is found
				httpRes.fileType =fileSupport[i].filetype;
				break;
			}
		}
		if(httpRes.fileType == NULL) {
			logger(sock,FORBIDDEN,"file extension type not supported",httpReq.uri);
			break;
		}
		asprintf(&httpRes.filePath,"../www%s",httpReq.uri);
	
		//2.7 URL Validation
		char resolved_path[PATH_MAX];
		char *ptr;
		ptr = realpath(httpRes.filePath, resolved_path);
		if(!ptr){
			logger(sock,NOTFOUND, "File not found - realpath",httpRes.filePath);
			break;
		}
	
		//simple request
		if(*(p+3)=='\0') {
			httpReq.isSimple=1;//it must respond with an HTTP/0.9 Simple-Response
			writeResponse(sock , httpRes);
			//Note that the Simple-Response consists only of the entity body and is terminated by the server closing the connection.
			close(sock);
			break;
		}
		//full request
		else {
			//decode HTTP version
			for (i=0; ; p++)
			{
				if(*p=='\r') {
					p+=2; // \r\n two characters
					break;
				}
				else if(*p=='\0'){
					logger(sock,BADREQUEST,"Bad Request...","");
					break;
				}
				httpReq.httpVersion[i++]=*p;
			}
			httpReq.httpVersion[i++]='\0'; //end of string
	/*				printf("URI: %s\n",httpReq.uri);*/
	/*				printf("HTTP version: %s\n",httpReq.httpVersion);*/
			printf("Message: %s\n",httpReq.message);
		
			writeResponse(sock, httpRes);
		}
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout); //print everything in the stdout buffer
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //Free the socket pointer
    free(mySocket);
     
    return 0;
}
