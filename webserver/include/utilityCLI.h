#ifndef UTILITYCLI_H
#define	UTILITYCLI_H

#include <stdio.h>
#include <stdlib.h>
#include "utilityHTTP.h"
#include "logger.h"
#include "daemon.h"

void printHelp();
void parseCommandLineOptions(ServerConfigurations *, int , char*[]);

#endif	/* UTILITYCLI_H */
