#ifndef UTILITYMANAGEFILES_H
#define UTILITYMANAGEFILES_H

#include<stdio.h>
#include <stdlib.h>
#include <unistd.h> // current directory
#include <syslog.h>
#include <errno.h>
#include "utilityHTTP.h"
#include "logger.h"

void parseConfigurationFile(ServerConfigurations *sc, char *fileName);

#endif
