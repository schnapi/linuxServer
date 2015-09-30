#ifndef DAEMON_H
#define DAEMON_H

#include "apue.h"
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>

void daemonize(const char *cmd);

#endif
