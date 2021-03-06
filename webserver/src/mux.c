#include "../include/mux.h"

#define MAXCLIENTS 400

int mux(int serverSocket) {
    //we need this to get client ip address
    struct sockaddr_in client;
    int c = sizeof (struct sockaddr_in);

    int acceptedSocket, endServer, squeezeArray, closeConnection; // all is 0 or FALSE
    //poll of sockets
    struct pollfd fds[MAXCLIENTS];
    // array of clients foreach file descriptor
    Client clients[MAXCLIENTS];
    //numberOfFileDescriptors: current number of file descriptors sockets
    int numberOfFileDescriptors = 1, currentSize, i, j, readSize; //i,j indexes for loops, readSize for recv function

    //initialize the pollfd structure
    memset(fds, 0, sizeof (fds));

    fcntl(serverSocket, F_SETFL, O_NONBLOCK);

    //set initial listening socket 
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN; //POLLIN data to read without blocking

    while (endServer == FALSE) {
        //printf("numberOfFileDescriptors: %d\n", numberOfFileDescriptors);
        //poll blocks and wait for clients
        if (poll(fds, numberOfFileDescriptors, -1) < 0) {
            loggerServer(LOG_ERR, "Poll function failed", "", NULL);
            break;
        }
        //we need to find which descriptor is readable
        currentSize = numberOfFileDescriptors;
        for (i = 0; i < currentSize; i++) {
            //this file descriptor is not ready yet
            if (fds[i].revents == 0) {
                continue;
            }

            //If revents is not ready to read data then it is an unexpected result
            if (fds[i].revents != POLLIN && fds[i].revents != POLLOUT) {
                loggerServer(LOG_ERR, "Error! revents", "", NULL);
                printf("Error! revents=%d,%d,%d,%d\n", fds[i].revents, POLLIN, POLLPRI, POLLOUT);
                endServer = TRUE;
                break;
            }
            //if server descriptor is readable
            if (fds[i].fd == serverSocket) {

                //accept all connections that are in queue and send data
                do {
                    if (numberOfFileDescriptors == MAXCLIENTS - 1) {
                        break;
                    }
                    //accept connections
                    acceptedSocket = accept(serverSocket, (struct sockaddr *) &client, (socklen_t*) & c);
                    //if accept fails, try again later or end server
                    if (acceptedSocket < 0) {
                        if (errno != EWOULDBLOCK) {
                            loggerServer(LOG_ERR, "Accept failed", "", NULL);
                            endServer = TRUE;
                        }
                        //if there is no clients anymore break and wait on poll
                        break;
                    }

                    //set client ip address
                    inet_ntop(AF_INET, &(client.sin_addr), clients[i].httpRes.IPAddress, INET_ADDRSTRLEN);

                    loggerServer(LOG_NOTICE, "Connection accepted", "", clients[i].httpRes.IPAddress);
                    //add client in queue
                    fds[numberOfFileDescriptors].fd = acceptedSocket;
                    fds[numberOfFileDescriptors].events = POLLOUT; //ready to write data, response is in revents flag
                    numberOfFileDescriptors++;
                } while (acceptedSocket != -1);
            } else {
                //send data to clients
                fcntl(fds[i].fd, F_SETFL, O_NONBLOCK);
                //loggerServer(LOG_NOTICE, "Handler assigned", "", clients[i].httpRes.IPAddress);
                closeConnection = FALSE;
                while (TRUE) {
                    //if receive fails
                    if ((readSize = recv(fds[i].fd, clients[i].httpReq.message, 10000, 0)) < 0) {
                        //if receive fail, but if there is no data, it returns EWOULDBLOCK
                        if (errno != EWOULDBLOCK) {
                            loggerServer(LOG_ERR, "Recv failed", "", clients[i].httpRes.IPAddress);
                            closeConnection = TRUE;
                        }
                        break;
                    }

                    //if client has closed connection
                    if (readSize == 0) {
                        closeConnection = TRUE;
                        break;
                    }
                    //parse received data
                    parseMessageSendResponse(fds[i].fd, &clients[i], readSize);

                    closeConnection = TRUE;
                    break;
                }

                //close connection and set fd to -1 that we can remove this fd from array
                if (closeConnection) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    //we want to squeeze array
                    squeezeArray = TRUE;
                }
            }
        }

        if (squeezeArray) {
            for (i = 0; i < numberOfFileDescriptors; i++) {
                // if is closed socket
                if (fds[i].fd == -1) {
                    //move all fd and clients on the rigth to the left
                    for (j = i; j < numberOfFileDescriptors; j++) {
                        fds[j] = fds[j + 1];
                        clients[j] = clients[j + 1];
                    }
                    //move one field back
                    i--;
                    //and decrease number of open sockets
                    numberOfFileDescriptors--;
                }
            }
            squeezeArray = FALSE;
        }
    }

    // close all the sockets that are open 
    for (i = 0; i < numberOfFileDescriptors; i++) {
        if (fds[i].fd >= 0) {
            close(fds[i].fd);
        }
    }

    return 0;
}
