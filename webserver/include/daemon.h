#ifndef DAEMON_H
#define DAEMON_H

#include "apue.h"
#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

void daemonize(const char *cmd);
int lockfile(int fd);
int already_running(void);

#endif
