
#include "../include/mux.h"

int mux(int listen_sd)
{
//we need this to get client ip address
	struct sockaddr_in client;
	int c = sizeof(struct sockaddr_in);
  int    rc = 1;
  int    clientSocket = -1;
  int    end_server = FALSE, compress_array = FALSE;
  int    close_conn;
  //time in miliseconds
  int    timeout;
  //poll of sockets
  struct pollfd fds[200];
  Client clients[200];
  //nfds: number of file descriptors sockets
  int    nfds = 1, current_size = 0, i, j;

  //initialize the pollfd structure
  memset(fds, 0 , sizeof(fds));

	//set initial listening socket 
  fds[0].fd = listen_sd;
  fds[0].events = POLLIN | POLLOUT; //POLLIN data to read, POLLOUT - write...
  /*************************************************************/
  
  timeout = 30000; //wait max 3 seconds if no response close connection

  /*************************************************************/
  /* Loop waiting for incoming connects or for incoming data   */
  /* on any of the connected sockets.                          */
  /*************************************************************/
  do
  {
    /***********************************************************/
    /* Call poll() and wait 3 minutes for it to complete.      */
    /***********************************************************/
    printf("Waiting on poll()...\n");
    rc = poll(fds, nfds, timeout);

    /***********************************************************/
    /* Check to see if the poll call failed.                   */
    /***********************************************************/
    if (rc < 0)
    {
      perror("  poll() failed");
      break;
    }

    /***********************************************************/
    /* Check to see if the 3 minute time out expired.          */
    /***********************************************************/
    if (rc == 0)
    {
      printf("  poll() timed out.  End program.\n");
      break;
    }


    //we need to find which descriptor is readable  
    current_size = nfds;
    for (i = 0; i < current_size; i++)
    {
      /*********************************************************/
      /* Loop through to find the descriptors that returned    */
      /* POLLIN and determine whether it's the listening       */
      /* or the active connection.                             */
      /*********************************************************/
      if(fds[i].revents == 0)
        continue;
		
      //If revents is not POLLIN or POLLOUT then it is an unexpected result
      if(fds[i].revents != POLLIN)
      {
        printf("  Error! revents = %d\n", fds[i].revents);
        end_server = TRUE;
        break;

      }
      if (fds[i].fd == listen_sd)
      {
        /*******************************************************/
        /* Listening descriptor is readable.                   */
        /*******************************************************/
        printf("  Listening socket is readable\n");

        /*******************************************************/
        /* Accept all incoming connections that are            */
        /* queued up on the listening socket before we         */
        /* loop back and call poll again.                      */
        /*******************************************************/
        do
        {
          /*****************************************************/
          /* Accept each incoming connection. If               */
          /* accept fails with EWOULDBLOCK, then we            */
          /* have accepted all of them. Any other              */
          /* failure on accept will cause us to end the        */
          /* server.                                           */
          /*****************************************************/
          clientSocket = accept(listen_sd, (struct sockaddr *)&client, (socklen_t*)&c);
          if (clientSocket < 0)
          {
            if (errno != EWOULDBLOCK)
            {
              perror("  accept() failed");
              end_server = TRUE;
            }
            break;
          }
          
          //set client ip address
		inet_ntop(AF_INET, &(client.sin_addr), clients[i-1].httpRes.IPAddress, INET_ADDRSTRLEN);
          printf("test: %s\n",clients[i-1].httpRes.IPAddress);

          /*****************************************************/
          /* Add the new incoming connection to the            */
          /* pollfd structure                                  */
          /*****************************************************/
          printf("  New incoming connection - %d\n", clientSocket);
          fds[nfds].fd = clientSocket;
          fds[nfds].events = POLLIN;
          nfds++;

          /*****************************************************/
          /* Loop back up and accept another incoming          */
          /* connection                                        */
          /*****************************************************/
        } while (clientSocket != -1);
      }

      /*********************************************************/
      /* This is not the listening socket, therefore an        */
      /* existing connection must be readable                  */
      /*********************************************************/

      else
      {
        printf("  Descriptor %d is readable\n", fds[i].fd);
        close_conn = FALSE;
        /*******************************************************/
        /* Receive all incoming data on this socket            */
        /* before we loop back and call poll again.            */
        /*******************************************************/

        do
        {
          /*****************************************************/
          /* Receive data on this connection until the         */
          /* recv fails with EWOULDBLOCK. If any other         */
          /* failure occurs, we will close the                 */
          /* connection.                                       */
          /*****************************************************/
          if((rc = recv(fds[i].fd, clients[i-1].httpReq.message, 10000, 0)) < 0)
          {
          	//if reads fail, but if there is no data, it returns EWOULDBLOCK 
          	//check them both
            if (errno != EWOULDBLOCK || errno != EAGAIN)
            {
              perror("  recv() failed");
              close_conn = TRUE;
            }
            break;
          }

          /*****************************************************/
          /* Check to see if the connection has been           */
          /* closed by the client                              */
          /*****************************************************/
          if (rc == 0)
          {
          	loggerServer(LOG_NOTICE,"Connection closed", "", clients[i-1].httpRes.IPAddress);
            printf("  Connection closed\n");
            close_conn = TRUE;
            break;
          }
		    if(parseMessageSendResponse(fds[i].fd,&clients[i-1])<0)
			{
				close(fds[i].fd);
            	close_conn = TRUE;
				break;
			}

        } while(TRUE);

        /*******************************************************/
        /* If the close_conn flag was turned on, we need       */
        /* to clean up this active connection. This            */
        /* clean up process includes removing the              */
        /* descriptor.                                         */
        /*******************************************************/
        if (close_conn)
        {
          close(fds[i].fd);
          fds[i].fd = -1;
          compress_array = TRUE;
        }


      }  /* End of existing connection is readable             */
    } /* End of loop through pollable descriptors              */

    /***********************************************************/
    /* If the compress_array flag was turned on, we need       */
    /* to squeeze together the array and decrement the number  */
    /* of file descriptors. We do not need to move back the    */
    /* events and revents fields because the events will always*/
    /* be POLLIN in this case, and revents is output.          */
    /***********************************************************/
    if (compress_array)
    {
      compress_array = FALSE;
      for (i = 0; i < nfds; i++)
      {
        if (fds[i].fd == -1)
        {
          for(j = i; j < nfds; j++)
          {
            fds[j].fd = fds[j+1].fd;
          }
          i--;
          nfds--;
        }
      }
    }

  } while (end_server == FALSE); /* End of serving running.    */

  /*************************************************************/
  /* Clean up all of the sockets that are open                 */
  /*************************************************************/
  for (i = 0; i < nfds; i++)
  {
    if(fds[i].fd >= 0)
      close(fds[i].fd);
  }
  return 0;
}
