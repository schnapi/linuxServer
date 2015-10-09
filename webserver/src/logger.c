#include "../include/logger.h"

int checkErrno(int socket, Client *client) {
    /*		printf("test1: %d\n",EACCES);*/
    /*		printf("test2: %d\n",EINVAL);*/
    /*		printf("test3: %d\n",EIO);*/
    /*		printf("test4: %d\n",ELOOP);*/
    /*		printf("test5: %d\n",ENAMETOOLONG);*/
    /*		printf("test6: %d\n",ENOMEM);*/
    /*		printf("test7: %d\n",ENOENT);*/
    /*		printf("test8: %d\n",ENOTDIR);*/
    if (errno != 0)
        printf("Errno is: %d\n", errno);
    switch (errno) {
        case EACCES:
            loggerClient(socket, FORBIDDEN, client, "Read or search permission was denied for a component of the path prefix.", client->httpRes.filePath);
            break;
        case EINVAL:
            loggerClient(socket, NOTFOUND, client, "File not found - realpath", client->httpRes.filePath);
            break;
        case EIO:
            loggerClient(socket, INTERNALSERVERERROR, client, "An I/O error occurred while reading from the filesystem.", client->httpRes.filePath);
            break;
        case ELOOP:
            loggerClient(socket, BADREQUEST, client, "Too many symbolic links were encountered in translating the pathname.", client->httpRes.filePath);
            break;
        case ENAMETOOLONG:
            loggerClient(socket, BADREQUEST, client, "A component of a pathname exceeded NAME_MAX characters, or an entire pathname exceeded PATH_MAX characters.", client->httpRes.filePath);
            break;
        case ENOMEM:
            loggerClient(socket, INTERNALSERVERERROR, client, "Out of memory.", client->httpRes.filePath);
            break;
        case ENOENT:
            loggerClient(socket, NOTFOUND, client, "The named file does not exist.", client->httpRes.filePath);
            break;
        case ENOTDIR:
            loggerClient(socket, BADREQUEST, client, "A component of the path prefix is not a directory.", client->httpRes.filePath);
            break;
    }
    return errno;
}

void writeToLogFile(char* filePath, char *logMessage, int error) {
    char* path;
    asprintf(&path, "/%s.log", filePath);
    int fp; // file pointer
    //a-append data
    // if file not exist create it
    if ((fp = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644)) >= 0) {
        write(fp, logMessage, strlen(logMessage));
        close(fp);
    } else
        syslog(LOG_ERR, "Write to log file failed: /%s", filePath);
    free(path);
    //here are errors and important notifications
    if (error) {
        asprintf(&path, "/%s.err", filePath);
        if ((fp = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644)) >= 0) {
            write(fp, logMessage, strlen(logMessage));
            close(fp);
        }
        free(path);
    }
}

void loggerServer(int level, char *s1, char *s2, char* clientIp) {
    char *levelString;
    switch (level) {
        case LOG_EMERG:
            levelString = "emergency";
            break;
        case LOG_ALERT:
            levelString = "alert";
            break;
        case LOG_CRIT:
            levelString = "critical";
            break;
        case LOG_ERR:
            levelString = "error";
            break;
        case LOG_WARNING:
            levelString = "warning";
            break;
        case LOG_NOTICE:
            levelString = "notice";
            break;
        case LOG_INFO:
            levelString = "info";
            break;
        case LOG_DEBUG:
            levelString = "debug";
            break;
    }
    char *logMessage;
    getDateTimeLog(dateTimeLog, SIZEOFDATETIMELOG);
    //if server error
    if (clientIp == NULL) {
        asprintf(&logMessage, "[%s] [%s] %s %s\n", dateTimeLog, levelString, s1, s2);
    }        //else client error
    else {
        asprintf(&logMessage, "[%s] [%s] [client %s] %s %s\n", dateTimeLog, levelString, clientIp, s1, s2);
    }
    //if is syslog	
    if (sc.customLog == NULL) {
        syslog(level, "%s", logMessage);
    } else {
        //if error has occured, or any important notification, look above codes from syslog
        if (level <= 3)
            writeToLogFile(sc.customLog, logMessage, 1);
        else
            writeToLogFile(sc.customLog, logMessage, 0);
    }
    free(logMessage);
}

void loggerClient(int socket, int method, Client *client, char *s1, char *s2) {
    switch (method) {
        case BADREQUEST:
            asprintf(&client->httpRes.filePath, "/%s/%d", sc.statusCodesDir, BADREQUEST);
            client->httpRes.statusCode = "400 Bad Request";
            break;
        case FORBIDDEN:
            asprintf(&client->httpRes.filePath, "/%s/%d", sc.statusCodesDir, FORBIDDEN);
            client->httpRes.statusCode = "403 Forbidden";
            break;
        case NOTFOUND:
            asprintf(&client->httpRes.filePath, "/%s/%d", sc.statusCodesDir, NOTFOUND);
            client->httpRes.statusCode = "404 Not Found";
            break;
        case INTERNALSERVERERROR:
            asprintf(&client->httpRes.filePath, "/%s/%d", sc.statusCodesDir, INTERNALSERVERERROR);
            client->httpRes.statusCode = "500 Internal Server Error";
            break;
        case NOTIMPLEMENTED:
            asprintf(&client->httpRes.filePath, "/%s/%d", sc.statusCodesDir, NOTIMPLEMENTED);
            client->httpRes.statusCode = "501 Not Implemented";
            break;
    }
    writeResponse(socket, client);
    free(client->httpRes.filePath);
}

void loggerSuccess(char* method, Client *client) {
    //date time int Common logInfo format for logger
    getDateTimeCLF(dateTimeCLF, SIZEOFDATETIMECLF);
    if (sc.customLog == NULL) {
        syslog(LOG_INFO, "%s - - [%s] \"%s %s %s\" %s %lu\n", client->httpRes.IPAddress, dateTimeCLF, client->httpReq.method, client->httpReq.uri, client->httpReq.httpVersion, method, client->httpRes.contentLength);
    } else {
        char *logMessage;
        //creates log message in CLF (Common logFile format)
        asprintf(&logMessage, "%s - - [%s] \"%s %s %s\" %s %lu\n", client->httpRes.IPAddress, dateTimeCLF, client->httpReq.method, client->httpReq.uri, client->httpReq.httpVersion, method, client->httpRes.contentLength);
        writeToLogFile(sc.customLog, logMessage, 0);
        //free memory due of use asprintf function
        free(logMessage);
    }
}



