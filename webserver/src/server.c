#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include <unistd.h> //read and write
#include <syslog.h>

#include<pthread.h> //for threading , link with lpthread
#include <fcntl.h> //for multiplexing
#include <limits.h> //PATH_MAX

#define SERVER
#include "../include/utilityHTTP.h"
#include "../include/utilityManageFiles.h"
#include "../include/utilityCLI.h"
#include "../include/logger.h"
#include "../include/mux.h"
#include "../include/parse.h"
#include "../include/daemon.h"


/*check the tasks file*/

void *connection_handler(void *); //pthread
extern int already_running(void);

typedef struct {
    int socket;
    char IpAddress[INET_ADDRSTRLEN];
} ClientSocket;
	
int main(int argc, char *argv[]) {
		
  parseConfigurationFile(&sc, ".lab3-config"); //utilityManageFiles.c
  sc.customLog = "log";
  sc.handlingMethod = "thread";
  
  parseCommandLineOptions(&sc, argc, argv);
  
  //set status code directory
	asprintf(&sc.statusCodesDir,"%s/%s",sc.executionDirectory,"statusCodesPages");
	

  //(however, you may choose to output to separate files, e.g. <filename>.log and <filename>.err)
  if (sc.customLog == NULL) {
		if(sc.isDaemon == 1){
			openlog("RoSa/1.0", LOG_CONS | LOG_PID, LOG_DAEMON);
		}
		else
      openlog("RoSa/1.0", LOG_CONS | LOG_PID, LOG_LOCAL1);
  }
  
	char *number;
  asprintf(&number, "%d", getuid());
  loggerServer(LOG_NOTICE, "Server RoSa started by User", number, NULL);
  free(number);
  
	if(sc.isDaemon == 1){
		asprintf(&number, "%d", getpid());
		loggerServer(LOG_NOTICE, "Server RoSa is running like a Daemon, PID: ", number, NULL);
		free(number);
	}

  //Create a new socket
  //Address Family - AF_INET (this is IP version 4) Type - SOCK_STREAM (this means connection oriented TCP protocol, SOCK_DGRAM is for UDP protocol) Protocol - 0 [ or IPPROTO_IP This is IP protocol]
  int mySocket = socket(AF_INET, SOCK_STREAM, 0);
  if (mySocket < 0) {
      loggerServer(LOG_ERR, "Could not create a socket", "", NULL);
      return 1;
  }

  struct sockaddr_in server, client;
  //sockaddr_in structure
  server.sin_family = AF_INET; //IP version 4
  server.sin_addr.s_addr = INADDR_ANY; // we can receive packets from any of the network interface card installed in the system
  server.sin_port = htons(sc.port); // port number

  int enable = 1; //enable options - nonzero
  //SO_REUSEADDR - reuse of local address
  if (setsockopt(mySocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof (int)) < 0) {
      loggerServer(LOG_ERR, "setsockopt", "", NULL);
      close(mySocket);
      return 1;
  }

  //Synchronous I/O Multiplexing - set socket to be non-blocking
  if (!strncmp(sc.handlingMethod, "mux", 3)) {
      fcntl(mySocket, F_SETFL, O_NONBLOCK);
  }

  //Bind a socket - listen to connections that are comming 
  if (bind(mySocket, (struct sockaddr *) &server, sizeof (server)) < 0) {
      loggerServer(LOG_ERR, "bind failed", "", NULL);
      close(mySocket);
      return 1;
  }
  loggerServer(LOG_NOTICE, "bind done", "", NULL);

  //http://pubs.opengroup.org/onlinepubs/9699919799/functions/listen.html
  if (listen(mySocket, 32) < 0) {
      loggerServer(LOG_ERR, "listen failed", "", NULL);
      close(mySocket);
      return 1;
  }

  //Accept and incoming connection
  asprintf(&number, "%d", sc.port);
  loggerServer(LOG_NOTICE, "Waiting for incoming connections on port:", number, NULL);
  free(number);
  loggerServer(LOG_NOTICE, "Chosen handling method is:", sc.handlingMethod, NULL);
  loggerServer(LOG_DEBUG, "www root directory is:", sc.rootDirectory, NULL);

  //mux - multiplexing synchronous I/O
  if (!strncmp(sc.handlingMethod, "mux", 3)) {
      return mux(mySocket);
  }

  int c = sizeof (struct sockaddr_in), clientSocket;
  ClientSocket *clientSocketP;
  //this could be a problem with accept if there is a lot of clients - better choice is synchronous I/O multiplexing
  while ((clientSocket = accept(mySocket, (struct sockaddr *) &client, (socklen_t*) & c))) {
      pthread_t clientThread;
      clientSocketP = malloc(1);
      //get ip
      inet_ntop(AF_INET, &(client.sin_addr), clientSocketP->IpAddress, INET_ADDRSTRLEN);
      loggerServer(LOG_NOTICE, "Connection accepted", "", clientSocketP->IpAddress);
      clientSocketP->socket = clientSocket;
      if (!strncmp(sc.handlingMethod, "thread", 6)) {
          //connection_handler function must return void* and take a single void* parameter
          // NULL means that the thread is created with default attributes
          if (pthread_create(&clientThread, NULL, connection_handler, (void*) clientSocketP) < 0) {
              loggerServer(LOG_ERR, "could not create thread", "", clientSocketP->IpAddress);
              return 1;
          }
          
          //do not join thread because we would have to wait this thread
          /*pthread_join( clientThread , NULL);*/
          loggerServer(LOG_NOTICE, "Handler assigned", "", clientSocketP->IpAddress);
          pthread_detach(clientThread);
      } else if (!strncmp(sc.handlingMethod, "fork", 4)) {
          puts("Not implemented");
          return 3;
      } else if (!strncmp(sc.handlingMethod, "prefork", 7)) {
          puts("Not implemented");
          return 3;
      }
  }

  if (clientSocket < 0) {
      loggerServer(LOG_ERR, "Accept failed", "", NULL);
      return 1;
  }

  if (sc.customLog == NULL) {
      closelog();
  }
  
  free(sc.executionDirectory);
  free(sc.statusCodesDir);

  return 0;
}

//This will handle connection for each client

void *connection_handler(void *mySocket) {
  ClientSocket* clientSocketP = (ClientSocket*) mySocket;
  int readSize;
  Client client;
  client.httpRes.fileType = NULL;
  strncpy(client.httpRes.IPAddress, clientSocketP->IpAddress, INET_ADDRSTRLEN);

  //Receive a message from client TCP, recvfrom is for udp
  while ((readSize = recv(clientSocketP->socket, client.httpReq.message, 10000, 0)) > 0) {
      parseMessageSendResponse(clientSocketP->socket, &client, readSize);
			
/*      if (client.httpRes.closeConnection == 1) {*/
          close(clientSocketP->socket);
          break;
/*      }*/
  }

  if (readSize == 0) {
      loggerServer(LOG_NOTICE, "Client disconnected", "", client.httpRes.IPAddress);
      fflush(stdout); //print everything in the stdout buffer
  } else if (readSize < 0) {
      loggerServer(LOG_ERR, "Recv failed", "", client.httpRes.IPAddress);
  }

  //Free the socket pointer
  free(mySocket);
  pthread_exit(NULL);
  return 0;
}
