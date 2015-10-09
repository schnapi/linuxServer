#include "../include/utilityManageFiles.h"

void parseConfigurationFile(ServerConfigurations *sc, char *fileName) {
    // we get current directory for configuration file
    char cwd[1024];
    if (getcwd(cwd, sizeof (cwd)) != NULL) {
        sc->executionDirectory = (char *) malloc(strlen(cwd));
        strcat(sc->executionDirectory, cwd);
    } else {
        loggerServer(LOG_ERR, "getcwd() error", "", NULL);
        exit(1);
    }
    FILE *stream;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    asprintf(&fileName, "%s/%s", sc->executionDirectory, fileName);
    stream = fopen(fileName, "r");
    if (stream == NULL) {
        loggerServer(LOG_ERR, "cannot open file in utilityManageFiles.c", "", NULL);
        exit(1);
    }
    //reads conf file
    while ((read = getline(&line, &len, stream)) != -1) {

        if (!strncmp(&line[0], "port", 4)) {
            char *port;
            asprintf(&port, "%s", line + 6);
            sc->port = atoi(port);
            if (sc->port == 0) {
                loggerServer(LOG_ERR, "Port number in configuration file is invalid.", "", NULL);
                exit(1);
            }
            free(port);
        } else if (!strncmp(&line[0], "rootDirectory", 13)) {
            asprintf(&(sc->rootDirectory), "%s", line + 15); //step over space
            sc->rootDirectory[strlen(sc->rootDirectory) - 1] = '\0'; //removes last new line character
            if (strlen(sc->executionDirectory) == strlen(sc->rootDirectory)) {
                if (!strncmp(sc->executionDirectory, sc->rootDirectory, strlen(sc->executionDirectory))) {
                    loggerServer(LOG_ERR, "Server’s execution directory and Document root directory are the same", "", NULL);
                    exit(1);
                }
            }
        } else if (!strncmp(&line[0], "handlingMethod", 14)) {
            asprintf(&(sc->handlingMethod), "%s", line + 16);
            sc->handlingMethod[strlen(sc->handlingMethod) - 1] = '\0'; //removes last new line character
        }
    }

    free(line);
    fclose(stream);
}
