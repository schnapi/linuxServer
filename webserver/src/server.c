#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include <unistd.h> //read and write
#include <syslog.h>
 
#include<pthread.h> //for threading , link with lpthread
#include <limits.h> //PATH_MAX
 
#define SERVER
#include "../include/utilityHTTP.h"
#include "../include/utilityManageFiles.h"
#include "../include/logger.h"


/*check the tasks file*/

void *connection_handler(void *);

int main(int argc , char *argv[])
{     
	parseConfigurationFile(&sc, ".lab3-config"); //utilityManageFiles.c

	//(however, you may choose to output to separate files, e.g. <filename>.log and <filename>.err)
	//sc.customLog = "log.log";
	if(sc.customLog==NULL) {
		openlog ("RoSa/1.0", LOG_CONS | LOG_PID, LOG_LOCAL1);
	}
	char* number;
	asprintf(&number,"%d", getuid());
	loggerServer(LOG_NOTICE,"Server RoSa started by User",number,NULL);
	free(number);
	
    //Create a new socket
	//Address Family - AF_INET (this is IP version 4) Type - SOCK_STREAM (this means connection oriented TCP protocol, SOCK_DGRAM is for UDP protocol) Protocol - 0 [ or IPPROTO_IP This is IP protocol]
    int mySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mySocket == -1)
    {
		loggerServer(LOG_ERR,"Could not create a socket","",NULL);
    }
     
    struct sockaddr_in server , client;
    //sockaddr_in structure
    server.sin_family = AF_INET; //IP version 4
    server.sin_addr.s_addr = INADDR_ANY; // we can receive packets from any of the network interface card installed in the system
    server.sin_port = htons( sc.port ); // port number
    
	int enable = 1; //enable options - nonzero
	//SO_REUSEADDR - reuse of local address
	if ( setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1 )
	{
		loggerServer(LOG_ERR,"setsockopt","",NULL);
	}
    //Bind a socket - listen to connections that are comming 
    if( bind(mySocket,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
		loggerServer(LOG_ERR,"bind failed","",NULL);
        return 1;
    }
	loggerServer(LOG_NOTICE,"bind done","",NULL);
     
    //http://pubs.opengroup.org/onlinepubs/9699919799/functions/listen.html
    listen(mySocket , 3);
     
    //Accept and incoming connection
	asprintf(&number,"%d", sc.port);
	loggerServer(LOG_NOTICE,"Waiting for incoming connections on port:",number,NULL);
	free(number);
	loggerServer(LOG_NOTICE,"Chosen handling method is:",sc.handlingMethod,NULL);
	loggerServer(LOG_DEBUG,"www root directory is:",sc.rootDirectory,NULL);
    int c = sizeof(struct sockaddr_in), clientSocket, *clientSocketP;
    while( (clientSocket = accept(mySocket, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
    //set ip
		loggerServer(LOG_NOTICE,"Connection accepted","",NULL);
		if(!strncmp(sc.handlingMethod,"thread",6)) {
			pthread_t sniffer_thread;
			clientSocketP = malloc(1);
			*clientSocketP = clientSocket;
			
			if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) clientSocketP) < 0)
			{
				loggerServer(LOG_ERR,"could not create thread","",NULL);
				return 1;
			}
			 
			//Now join the thread , so that we dont terminate before the thread
			pthread_join( sniffer_thread , NULL);
			loggerServer(LOG_NOTICE,"Handler assigned","",NULL);
		}
		else if(!strncmp(sc.handlingMethod,"fork",4)) {
			puts("Not implemented");
			return 3;
		}
		else if(!strncmp(sc.handlingMethod,"prefork",7)) {
			puts("Not implemented");
			return 3;
		}
		else if(!strncmp(sc.handlingMethod,"mux",3)) {
			puts("Not implemented");
			return 3;
		}
    }
     
    if (clientSocket<0)
    {
		loggerServer(LOG_ERR,"Accept failed","",NULL);
        return 1;
    }
    
	if(sc.customLog==NULL) {
    	closelog();
	}
     
    return 0;
}

void writeResponse(int socket, Client *client){
	FILE *fp;
	//r-read data
	if((fp = fopen(client->httpRes.filePath, "r")) == NULL) {
		//error can't open file or file not found // already is checked with realpath 
		//permisions denied
		checkErrno(socket,client);
	}
	else {
		//if request is not simple then send headers
		if(!client->httpRes.simple) {
			fseek(fp, 0L, SEEK_END); //goes to end of the file
			client->httpRes.contentLength=ftell(fp);
			fseek(fp,0L,SEEK_SET); //go back to where we were
			client->httpRes.statusCode="200 OK";
			generateHeader(&client->httpRes); // in utilityHTTP.c
			/* Header + a blank line */
			write(socket,client->httpRes.buffer,strlen(client->httpRes.buffer));
		}
		if(!strncmp(client->httpReq.method,"GET",3)) {
			// send file in 4KB block
			int length;
			while (	(length = fread(client->httpRes.buffer, 1, BUFFERSIZE,fp)) > 0 ) {
				write(socket,client->httpRes.buffer,length);
			}
		}
		fclose(fp);
		logger(200, client);
	}
}

int validateURL(int sock,Client *client){
	//2.7 URL Validation
	asprintf(&client->httpRes.filePath,"%s%s",sc.rootDirectory,client->httpReq.uri);
	//checks overflow
	if(strlen(client->httpRes.filePath)>=PATH_MAX){
		return 0;
	}
	char resolved_path[PATH_MAX];
	realpath(client->httpRes.filePath, resolved_path);
	
	client->httpRes.filePath=resolved_path;

	if(checkErrno(sock,client)) {//check if any error has occured, 0 means that file exist
		return 0;
	}
	//file extension support
	int uriLength=strlen(client->httpReq.uri),extensionLength;
	int i=0;
	for(i=0;fileSupport[i].extension != 0;i++) {
		extensionLength = strlen(fileSupport[i].extension);
		//we get last characters from uri, -extensionLength
		// if they match
		if( strncmp(&client->httpReq.uri[uriLength-extensionLength], fileSupport[i].extension, extensionLength) == 0) {
			//fileType is found
			client->httpRes.fileType =fileSupport[i].filetype;
			return 1;
		}
	}
	return 0;
}
 /*
 * This will handle connection for each client
 * */
void *connection_handler(void *mySocket)
{
    //Get the socket descriptor
    int sock = *(int*)mySocket;
    int read_size;
    Client client;
    client.httpRes.fileType = NULL;
    // get ip from client
	char ipstr[INET_ADDRSTRLEN];
	bzero(ipstr, 50);
	struct sockaddr_in address;
	socklen_t address_len = sizeof(address);
	getpeername(sock, (struct sockaddr *) &address, &address_len);
	inet_ntop(AF_INET, &address.sin_addr, ipstr, sizeof(ipstr));
	client.httpRes.IPAddress = ipstr;
	
    //Receive a message from client
    while( (read_size = recv(sock , client.httpReq.message , 10000 , 0)) > 0 )
    {
        //Send the message back to client
		const char* p = &client.httpReq.message[0];
		int i=0;
		if(memcmp(p, "GET",3)==0){
			client.httpReq.method="GET";
			p+=4;//+1 space				
		}
		else if(memcmp(p, "HEAD",4)==0){
			//The HEAD method is identical to GET except that the server must not return any Entity-Body in the response. The metainformation contained in the HTTP headers in response to a HEAD request should be identical to the information sent in response to a GET request. This method can be used for obtaining metainformation about the resource identified by the Request-URI without transferring the Entity-Body itself. This method is often used for testing hypertext links for validity, accessibility, and recent modification. There is no "conditional HEAD" request analogous to the conditional GET. If an If-Modified-Since header field is included with a HEAD request, it should be ignored.
			client.httpReq.method="HEAD";
			p+=5;//+1 space
		}
		else {
			loggerClient(sock,NOTIMPLEMENTED,&client, "Only simple GET and HEAD operation supported","");
			break;
		}
			
		//decode request URI ???
		for (; ; p++)
		{
			//if is owerflow
			if(i==PATH_MAX) {
				loggerClient(sock,BADREQUEST,&client,"","");
				close(sock);
    			free(mySocket);
				return (void*)1;
			}
			else if(*p=='\r') {
				p+=2; // \r\n two characters
				break;
			}
			else if(*p==' '){
				p++;
				break;
			}
			// get uri
			client.httpReq.uri[i++]=*p;
		}
		client.httpReq.uri[i]='\0'; //end of string
		if(strcmp(client.httpReq.uri,"/") == 0) {
			strncpy(client.httpReq.uri, "/index.html", 11);
		}
	
		//simple request
		if(*(p+3)=='\0') {
			//if URL is not valid 
			if(!validateURL(sock,&client)){
				close(sock);
    			free(mySocket);
				return (void*)1;
			}
			client.httpRes.simple=1;//it must respond with an HTTP/0.9 Simple-Response
			writeResponse(sock , &client);
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
					loggerClient(sock,BADREQUEST,&client,"Bad Request...","");
					break;
				}
				client.httpReq.httpVersion[i++]=*p;
			}
			client.httpReq.httpVersion[i++]='\0'; //end of string
			
			//if URL is not valid 
			if(!validateURL(sock,&client)){
				close(sock);
    			free(mySocket);
				return (void*)1;
			}
	/*				printf("URI: %s\n",httpReq.uri);*/
	/*				printf("HTTP version: %s\n",httpReq.httpVersion);*/
			printf("Message: %s\n",client.httpReq.message);
		
			writeResponse(sock, &client);
		}
    }
     
    if(read_size == 0)
    {
		loggerServer(LOG_NOTICE,"Client disconnected","",client.httpRes.IPAddress);
        fflush(stdout); //print everything in the stdout buffer
    }
    else if(read_size == -1)
    {
		loggerServer(LOG_ERR,"Recv failer","",client.httpRes.IPAddress);
    }
         
    //Free the socket pointer
    free(mySocket);
     
    return 0;
}
