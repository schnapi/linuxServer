
#include "../include/parse.h"

void writeResponse(int socket, Client *client) {
    /*http://pubs.opengroup.org/onlinepubs/009695399/functions/flockfile.html*/
    FILE *fp;
    //r-read data
    if ((fp = fopen(client->httpRes.filePath, "r")) == NULL) {
        //error can't open file or file not found // already is checked with realpath 
        //permisions denied
        checkErrno(socket, client);
    } else {
        fseek(fp, 0L, SEEK_END); //goes to end of the file
        client->httpRes.contentLength = ftell(fp);
        fseek(fp, 0L, SEEK_SET); //go back to where we were
        //if request is not simple then send headers
        if (!client->httpRes.simple) {
            generateHeader(&client->httpRes); // in utilityHTTP.c
            /* Header + a blank line */
            write(socket, client->httpRes.buffer, strlen(client->httpRes.buffer));
        }
        //if is get request then send file
        if (!strncmp(client->httpReq.method, "GET", 3)) {
            // send file in 4KB block
            int length;
            while ((length = fread(client->httpRes.buffer, 1, BUFFERSIZE, fp)) > 0) {
                write(socket, client->httpRes.buffer, length);
            }
            client->httpRes.closeConnection = 1;
        } else {
            client->httpRes.closeConnection = 1;
        }
        fclose(fp);
        char statusCode[4];
        strncpy(statusCode, client->httpRes.statusCode, 3);
        statusCode[3] = '\0';
        //send success to logger
        loggerSuccess(statusCode, client);

    }
}

int validateURL(int sock, Client *client) {
    //2.7 URL Validation
    asprintf(&client->httpRes.filePath, "%s%s", sc.rootDirectory, client->httpReq.uri);
    //checks overflow
    if (strlen(client->httpRes.filePath) >= PATH_MAX) {
        loggerClient(sock, BADREQUEST, client, "A component of a pathname exceeded NAME_MAX characters, or an entire pathname exceeded PATH_MAX characters.", client->httpRes.filePath);
        return -1;
    }
    char resolved_path[PATH_MAX];
    realpath(client->httpRes.filePath, resolved_path);

    client->httpRes.filePath = resolved_path;
    if (checkErrno(sock, client)) {//check if any error has occured, 0 means that file exist
        return -1;
    }
    //file extension support
    int uriLength = strlen(client->httpReq.uri), extensionLength;
    int i = 0;
    for (i = 0; sc.fileSupport[i].extension != 0; i++) {
        extensionLength = strlen(sc.fileSupport[i].extension);
        //we get last characters from uri, -extensionLength
        // if they match
        if (strncmp(&client->httpReq.uri[uriLength - extensionLength], sc.fileSupport[i].extension, extensionLength) == 0) {
            //fileType is found
            client->httpRes.fileType = sc.fileSupport[i].filetype;
            return 0;
        }
    }
    //no extension support
    loggerClient(sock, FORBIDDEN, client, "Read or search permission was denied for a component of the path prefix.", client->httpRes.filePath);
    return -1;
}

void parseMessageSendResponse(int socket, Client *client, int readSize) {
    //set end of message due owerflow
    client->httpReq.message[readSize] = '\0';
    //set errno to null
    errno = 0;
    //Send the message back to client
    const char* p = &client->httpReq.message[0];
    int i = 0;
    if (memcmp(p, "GET", 3) == 0) {
        client->httpReq.method = "GET";
        p += 4; //+1 space	
        i += 4;
    } else if (memcmp(p, "HEAD", 4) == 0) {
        //The HEAD method is identical to GET except that the server must not return any Entity-Body in the response. The metainformation contained in the HTTP headers in response to a HEAD request should be identical to the information sent in response to a GET request. This method can be used for obtaining metainformation about the resource identified by the Request-URI without transferring the Entity-Body itself. This method is often used for testing hypertext links for validity, accessibility, and recent modification. There is no "conditional HEAD" request analogous to the conditional GET. If an If-Modified-Since header field is included with a HEAD request, it should be ignored.
        client->httpReq.method = "HEAD";
        p += 5; //+1 space
        i += 5;
    } else {
        loggerClient(socket, NOTIMPLEMENTED, client, "Only simple GET and HEAD operation supported", "");
        return;
    }
    //check if is wrong request
    if (readSize <= i) {
        loggerClient(socket, BADREQUEST, client, "Bad request", "");
        return;
    }

    //decode request URI ???
    for (i = 0;; p++) {
        //if is owerflow
        if (i == PATH_MAX) {
            loggerClient(socket, BADREQUEST, client, "", "");
            return;
        } else if (*p == '\r') {
            client->httpRes.simple = 1;
            break;
        } else if (*p == ' ') {
            p++;
            break;
        }
        // get uri
        client->httpReq.uri[i++] = *p;
    }
    if (readSize <= i) {
        loggerClient(socket, BADREQUEST, client, "Bad request", "");
        return;
    }
    //protection for owerflow
    client->httpReq.uri[i] = '\0'; //end of string
    if (client->httpReq.uri[strlen(client->httpReq.uri) - 1] == '/') {
        //Adds index.html if the requested path ends with '/'.
        char* uri;
        asprintf(&uri, "%sindex.html", client->httpReq.uri);
        strcpy(client->httpReq.uri, uri);
        free(uri);
    }
    //simple request
    if (client->httpRes.simple) {
        //if is wrong method
        if (strncmp(client->httpReq.method, "GET", 3)) {
            loggerClient(socket, BADREQUEST, client, "", "");
            return;
        }
        //if URL is not valid 
        if (validateURL(socket, client) < 0) {
            return;
        }
        //it must respond with an HTTP/0.9 Simple-Response
        strncpy(client->httpReq.httpVersion, "HTTP/0.9\0", 9);
        client->httpRes.statusCode = "200 OK";
        writeResponse(socket, client);
        //Note that the Simple-Response consists only of the entity body and is terminated by the server closing the connection.
        client->httpRes.closeConnection = 1;
    }        //full request
    else {
        //decode HTTP version
        for (i = 0; i < 8; p++) {
            if (*p == '\r') {
                p += 2; // \r\n two characters
                break;
            } else if (*p == '\0') {
                loggerClient(socket, BADREQUEST, client, "Bad Request...", "");
                break;
            }
            client->httpReq.httpVersion[i++] = *p;
        }
        //protection for owerflow
        client->httpReq.httpVersion[8] = '\0';

        //if URL is not valid 
        if (validateURL(socket, client) < 0) {
            return;
        }
        /*				printf("URI: %s\n",client->httpReq.uri);*/
        /*				printf("HTTP version: %s\n",client->httpReq.httpVersion);*/
        /*		printf("Message: %s\n",client->httpReq.message);*/

        client->httpRes.statusCode = "200 OK";
        writeResponse(socket, client);
    }
}
