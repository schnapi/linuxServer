#ifndef DAEMON_H
#define DAEMON_H

#include <sys/resource.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "utilityHTTP.h"
#include "logger.h"

void createDaemon(const char *programName);
int lockfile(int fd);
int daemonIsRunning();

#endif
