#include "../include/dateTime.h"

void getDateTimeGMT(char *buffer, size_t size) {

	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );

	timeinfo = gmtime ( &rawtime );

	strftime (buffer,size,"%a, %e %b %Y %X %Z",timeinfo);
}

void getDateTimeCLF(char *buffer, size_t size) {

	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );

	timeinfo = localtime ( &rawtime );

	strftime (buffer,size,"%d/%m/%Y:%T %z",timeinfo);
}

void getDateTimeLog(char *buffer, size_t size) {

	time_t rawtime;
	struct tm * timeinfo;

	time ( &rawtime );

	timeinfo = localtime ( &rawtime );

	strftime (buffer,size,"%b %d %T %Y",timeinfo);
}

void getFileCreationTime(char *buffer, size_t size, char *filePath)
{
    struct stat attrib;
	struct tm * timeinfo;
    stat(filePath, &attrib);
    
	timeinfo = gmtime(&attrib.st_mtime); //st_mtime   Time of last data modification. 
    strftime(buffer, size, "%a, %e %b %Y %X %Z", timeinfo);
}
