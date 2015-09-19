#include "../include/utilityManageFiles.h"

void parseConfigurationFile(ServerConfigurations *sc, char *fileName) {
	// we get current directory for configuration file
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	   fprintf(stdout, "Server’s execution directory dir: %s\n", cwd);
	else
	   perror("getcwd() error");
	FILE *stream;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;
	asprintf(&fileName,"%s/%s",cwd,fileName);
	stream = fopen(fileName, "r");
	if (stream == NULL)
	   exit(EXIT_FAILURE);
	//reads conf file
	while ((read = getline(&line, &len, stream)) != -1) {

		if(!strncmp(&line[0],"port",4)) {
			char *port;
			asprintf(&port,"%s",line+6);
			sc->port=atoi(port);
			if(sc->port == 0)
			{
				perror("Port number in configuration file is invalid.");
	   			exit(EXIT_FAILURE);
			}
			free(port);
		}
		else if(!strncmp(&line[0],"rootDirectory",13)){
			asprintf(&(sc->rootDirectory),"%s",line+15);//step over space
			sc->rootDirectory[strlen(sc->rootDirectory)-1]='\0';//removes last new line character
			if(strlen(cwd)==strlen(sc->rootDirectory)){
				if(!strncmp(cwd,sc->rootDirectory,strlen(cwd))){
					printf("Error: Server’s execution directory and Document root directory are the same:\n");
		   			exit(EXIT_FAILURE);
				}
			}
		}
		else if(!strncmp(&line[0],"handlingMethod",14)){
			asprintf(&(sc->handlingMethod),"%s",line+16);
			sc->handlingMethod[strlen(sc->handlingMethod)-1]='\0';//removes last new line character
		}
	}

	free(line);
	fclose(stream);
}
