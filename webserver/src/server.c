#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
 
#include<pthread.h> //for threading , link with lpthread
 
#include "logger.h"

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

char *getFile(char *sendFile){
	/*unsigned char send_buffer[MAX_LEN];

	while( !feof(sendFile) )
	{
		int numread = fread(send_buffer, sizeof(unsigned char), MAX_LEN, sendFile);
		if( numread < 1 ) break; // EOF or error

		unsigned char *send_buffer_ptr = send_buffer;
		do
		{
		    int numsent = send(new_fd, send_buffer_ptr, numread, 0);
		    if( numsent < 1 ) // 0 if disconnected, otherwise error
		    {
		        if( numsent < 0 )
		        {
		            if( WSAGetLastError() == WSAEWOULDBLOCK )
		            {
		                fd_set wfd;
		                FD_ZERO(&wfd);
		                FD_SET(new_fd, &wfd);

		                timeval tm;
		                tm.tv_sec = 10;
		                tm.tv_usec = 0;

		                if( select(0, NULL, &wfd, NULL, &tm) > 0 )
		                    continue;
		            }
		        }

		        break; // timeout or error
		    }

		    send_buffer_ptr += numsent;
		    numread -= numsent;
		}
		while( numread > 0 );
	} */
	return NULL;
}


typedef struct {
	char* statusCode;
	int http_status,keep_alive;
	struct Response {
		int keep_alive;
	} response;
} HTTPResponse;
typedef struct {
	char message[10000];
	char* method;
	char uri[10000];
	char httpVersion[8];
	short isSimple;
	char host[10000];
} HTTPRequest;

int writeResponse(int sock, HTTPResponse httpRes){
	char *statusLine=httpRes.statusCode;
	char *head="Cache-Control: no-cache, private\nContent-Length: 11\nDate: Mon, 24 Nov 2014 10:21:21 GMT\n\n\0"; //two new lines
	char *header = (char *) malloc(strlen(statusLine)+strlen(head));
	/* Initial memory allocation */
	const char* p = &statusLine[0];
	int i=0;
	for (; *p != '\0'; p++)
	{
		header[i++]=*p;
	}
	p = &head[0];
	for (; *p != '\0'; p++)
	{
		header[i++]=*p;
	}
	printf("%d\n",i);
	printf("\n%s",header);
	write(sock , header , i);
    char *content="hello Sandi";
	write(sock , content , strlen(content));
	
}

void *connection_handler(void *mySocket)
{
    //Get the socket descriptor
    int sock = *(int*)mySocket;
    int read_size;
    HTTPResponse httpRes;
    HTTPRequest httpReq;
    
    //Receive a message from client
    while( (read_size = recv(sock , httpReq.message , 10000 , 0)) > 0 )
    {
        //Send the message back to client
		const char* p = &httpReq.message[0];
		int i=0;
		if(memcmp(p, "GET",3)==0){
			httpReq.method="GET";
			p+=4;//+1 space		
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
			//simple request
			if(*(p+3)=='\0') {
				httpReq.isSimple=1;//it must respond with an HTTP/0.9 Simple-Response
				//Note that the Simple-Response consists only of the entity body and is terminated by the server closing the connection.
				char *content="Simple request";
		    	write(sock , content , strlen(content));
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
						//wrong request
						break;
					}
					httpReq.httpVersion[i++]=*p;
				}
				httpReq.httpVersion[i++]='\0'; //end of string
				printf("URI: %s\n",httpReq.uri);
				printf("HTTP version: %s\n",httpReq.httpVersion);
				for (i=0; *p != ' ' && *p != '\r'; p++)
				{
					httpReq.httpVersion[i++]=*p;
				}
			
				printf("%.*s", read_size, httpReq.message);
				httpRes.statusCode="HTTP/1.1 " "200 OK" "\n\0";
				writeResponse(sock, httpRes);
			}
			
		}
		else if(memcmp(p, "HEAD",4)==0){
			//The HEAD method is identical to GET except that the server must not return any Entity-Body in the response. The metainformation contained in the HTTP headers in response to a HEAD request should be identical to the information sent in response to a GET request. This method can be used for obtaining metainformation about the resource identified by the Request-URI without transferring the Entity-Body itself. This method is often used for testing hypertext links for validity, accessibility, and recent modification. There is no "conditional HEAD" request analogous to the conditional GET. If an If-Modified-Since header field is included with a HEAD request, it should be ignored.
			httpReq.method="HEAD";
			httpRes.statusCode="HTTP/1.1 " "200 OK" "\n\0";
			printf("HEAD it works\n");
			p+=4;
			char *statusLine=httpRes.statusCode;
			char *head="Cache-Control: no-cache, private\nContent-Length: 0\nDate: Mon, 24 Nov 2014 10:21:21 GMT\n\n\0"; //two new lines
	   		char *header = (char *) malloc(strlen(statusLine)+strlen(head));
			/* Initial memory allocation */
			p = &statusLine[0];
			i=0;
			for (; *p != '\0'; p++)
			{
				header[i++]=*p;
			}
			p = &head[0];
			for (; *p != '\0'; p++)
			{
				header[i++]=*p;
			}
			printf("%d\n",i);
			printf("\n%s",header);
			write(sock , header , i);
		}
		else {
			logger(501,"Only simple GET and HEAD operation supported","",sock);
			break;
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
