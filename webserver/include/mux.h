#ifndef MUX_H
#define MUX_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */

#define TRUE             1
#define FALSE            0

int mux(int listen_sd);

#endif
