#include "../include/mux.h"

int mux(int serverSocket)
{
	//we need this to get client ip address
	struct sockaddr_in client;
	int c=sizeof(struct sockaddr_in);
	
	int acceptedSocket, endServer, squeezeArray,close_conn; // all is 0 or FALSE
	//poll of sockets
	struct pollfd fds[200];
	// array of clients foreach file descriptor
	Client clients[200];
	//nfds: current number of file descriptors sockets
	int nfds=1, currentSize, i, j, readSize; //i,j indexes for loops, readSize for recv function

	//initialize the pollfd structure
	memset(fds, 0 , sizeof(fds));

	//set initial listening socket 
	fds[0].fd=serverSocket;
	fds[0].events=POLLIN; //POLLIN data to read without blocking

	while (endServer==FALSE)
	{
		printf("Waiting on clients.\n");
		//poll blocks and wait for clients
		if(poll(fds, nfds, -1)<0) {
			perror("Poll function failed");
			break;
		}

		//we need to find which descriptor is readable
		currentSize=nfds;
		for (i=0; i < currentSize; i++)
		{
			//this file descriptor is not ready yet
			if(fds[i].revents==0)
				continue;
	
			//If revents is not ready to read data then it is an unexpected result
			if(fds[i].revents!=POLLIN)
			{
					printf("Error! revents=%d\n", fds[i].revents);
					endServer=TRUE;
					break;
			}
			//if server descriptor is readable
			if (fds[i].fd==serverSocket)
			{
			 		printf("Listening socket is readable\n");

					//accept all connections that are in queue and send data
					do
					{
						//accept connections
						acceptedSocket=accept(serverSocket, (struct sockaddr *)&client, (socklen_t*)&c);
						//if accept fails, try again later or end server
						if (acceptedSocket < 0)
						{
							if (errno!=EWOULDBLOCK)
							{
								loggerServer(LOG_ERR,"Accept failed","",NULL);
								endServer=TRUE;
							}
							//if there is no clients anymore break and wait on poll
							break;
						}
			 
						//set client ip address
						inet_ntop(AF_INET, &(client.sin_addr), clients[i].httpRes.IPAddress, INET_ADDRSTRLEN);

						loggerServer(LOG_NOTICE,"Connection accepted","", clients[i].httpRes.IPAddress);
						//add client in queue
						fds[nfds].fd=acceptedSocket;
						fds[nfds].events=POLLIN; //ready to read data, response is in revents flag
					printf("ac: %d\n",fds[nfds].fd);
						nfds++;
					} while (acceptedSocket!=-1);
			}
			//send data to clients
			else
			{
				loggerServer(LOG_NOTICE,"Handler assigned","", clients[i-1].httpRes.IPAddress);
				close_conn=FALSE;
				while(TRUE)
				{
					printf("z: %d\n",fds[i].fd);
					printf("z: %d\n",errno);
					//if receive fails
					if((readSize=recv(fds[i].fd, clients[i-1].httpReq.message, 10000, 0)) < 0)
					{
					printf("test: %d\n",fds[i].fd);
					printf("test: %d\n",errno);
						//if receive fail, but if there is no data, it returns EWOULDBLOCK
						if (errno!=EWOULDBLOCK)
						{
							loggerServer(LOG_ERR,"Recv failed","",clients[i-1].httpRes.IPAddress);
							close_conn=TRUE;
						}
						break;
					}

					//if client has closed connection
					if (readSize==0)
					{
						close_conn=TRUE;
						break;
					}
					//parse received data
					parseMessageSendResponse(fds[i].fd,&clients[i-1], readSize);
					if(clients[i-1].httpRes.closeConnection == 1)
					{
						close(fds[i].fd);
						close_conn=TRUE;
						break;
					}
				}

				//close connection and set fd to -1 that we can remove this fd from array
				if (close_conn)
				{
					close(fds[i].fd);
					fds[i].fd=-1;
					//we want to squeeze array
					squeezeArray=TRUE;
				}
			}
		}
		
		if (squeezeArray)
		{
			for (i=0; i < nfds; i++)
			{
				// if is closed socket
				if (fds[i].fd==-1)
				{
					//move all fd and clients on the rigth to the left
					for(j=i; j < nfds; j++)
					{
						fds[j].fd=fds[j+1].fd;
						clients[j]=clients[j+1];
					}
					//move one field back
					i--;
					//and decrease number of open sockets
					nfds--;
				}
			}
			squeezeArray=FALSE;
		}
	}

	// close all the sockets that are open 
	for (i=0; i < nfds; i++)
	{
		if(fds[i].fd>=0){
			close(fds[i].fd);
		}
	}
	
	return 0;
}
