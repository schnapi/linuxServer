#include "../include/utilityCLI.h"

void parseCommandLineOptions(ServerConfigurations *sc, int argc, char* argv[]) {
    int i;
    // i = 1 to avoid argv[0], which is the name of the executable.
    for (i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-h", 2) == 0) {
            printHelp();
            exit(EXIT_SUCCESS);
        } else if (strncmp(argv[i], "-p", 2) == 0) {
            if ((argc - 1) == i) {
                printf("-p found but port missing...\n");
                exit(EXIT_FAILURE);
            } else {
                int port = atoi(argv[++i]);
                printf("-p - using port %i\n", port);
                sc->port = port;
            }
        } else if (strncmp(argv[i], "-d", 2) == 0) {
            //this is for daemon
            char *programName;
            //if slash character is missing then program name is argv[0]
            if ((programName = strrchr(argv[0], '/')) == NULL)
                programName = argv[0];
                //increase pointer by one to remove slash
            else
                programName++;

            createDaemon(programName);
            if (daemonIsRunning()) {
                loggerServer(LOG_ERR, "daemon already running", "", NULL);
                exit(1);
            } else {
                sc->isDaemon = 1;
            }
        } else if (strncmp(argv[i], "-l", 2) == 0) {
            if ((argc - 1) == i) {
                printf("-l found but filename is missing...\n");
                exit(EXIT_FAILURE);
            } else {
                char* filename = argv[++i];
                printf("-l - using file '%s'\n", filename);
                sc->customLog = filename;
            }
        } else if (strncmp(argv[i], "-s", 2) == 0) {
            if ((argc - 1) == i) {
                printf("-s found but method is missing...\n");
                exit(EXIT_FAILURE);
            } else if (
                    strncmp(argv[i + 1], "fork", 4) != 0 &&
                    strncmp(argv[i + 1], "thread", 6) != 0 &&
                    strncmp(argv[i + 1], "prefork", 7) != 0 &&
                    strncmp(argv[i + 1], "mux", 3) != 0
                    ) {
                printf("-s found but method is unknown...\n");
                exit(EXIT_FAILURE);
            } else {
                char* method = argv[++i];
                printf("-s - using method '%s'\n", method);
                sc->handlingMethod = method; // TODO: maybe use an enumeration for handling methods.
            }
        } else {
            printf("Unknown parameter '%s'\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
}

void printHelp() {
    printf("\n\t\tHELP\n\n");
    printf("-h\tPrint help text.\n");
    printf("-p port\tListen to port number port.\n");
    printf("-d\tRun as a daemon instead of as a normal program.\n");
    printf("-l logfile\tLog to logfile. If this option is not specified, logging will be output to syslog, which is the default.\n");
    printf("-s [fork | thread | prefork | mux]\tSelect request handling method.\n");
}
