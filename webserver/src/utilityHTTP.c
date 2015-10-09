#include "../include/utilityHTTP.h"

void generateHeader(HTTPResponse *httpRes) {
    getDateTimeGMT(dateTimeGMT, SIZEOFDATETIMEGMT);
    // if type is null we know that we don't read from file
    if (httpRes->fileType == NULL) {
        httpRes->fileType = "text/html";
    }
    getFileCreationTime(httpRes->lastModified, SIZEOFDATETIMEGMT, httpRes->filePath); //dateTime.c
    sprintf(httpRes->buffer,
            "HTTP/1.0 %s\r\nDate: %s\r\nLast-Modified: %s\r\nAllow: GET, HEAD\r\nServer: %s\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n",
            httpRes->statusCode,
            dateTimeGMT,
            httpRes->lastModified,
            SERVERNAME,
            httpRes->contentLength,
            httpRes->fileType);
}

void utilityHTTPFree(HTTPResponse *httpRes) {
    free(httpRes->buffer);
}
