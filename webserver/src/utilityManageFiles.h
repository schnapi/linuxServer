#ifndef UTILITYFILES_H
#define UTILITYFILES_H

#include<stdio.h>
#include <stdlib.h>
#include <unistd.h> // current directory
#include "utilityHTTP.h"

void parseConfigurationFile(ServerConfigurations *sc, char *fileName);

#endif
