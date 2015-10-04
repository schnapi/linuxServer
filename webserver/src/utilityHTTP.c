#include "../include/utilityHTTP.h"

void generateHeader(HTTPResponse *httpRes){
	getDateTimeGMT(dateTimeGMT,SIZEOFDATETIMEGMT);
	// if type is null we know that we don't read from file
	if(httpRes->fileType == NULL)
	{
		httpRes->fileType = "text/html";
	}
	getFileCreationTime(httpRes->lastModified, SIZEOFDATETIMEGMT, httpRes->filePath); //dateTime.c
	sprintf(httpRes->buffer,"HTTP/1.0 %s\nDate: %s\nLast-Modified: %s\nAllow: GET, HEAD\nServer: %s\nContent-Length: %ld\nContent-Type: %s\n\n",httpRes->statusCode, dateTimeGMT,httpRes->lastModified,SERVERNAME, httpRes->contentLength, httpRes->fileType);
}

void utilityHTTPFree(HTTPResponse *httpRes){
	free(httpRes->buffer);
}
