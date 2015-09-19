#ifndef DATETIME_H
#define DATETIME_H

#include<stdio.h>
#include <stdlib.h>
#include <sys/stat.h> // file stat
#include <time.h> /* time_t, struct tm, time, localtime, strftime */

void getDateTimeGMT(char *buffer, size_t size);
void getFileCreationTime(char *buffer, size_t size, char *filePath);

#endif
