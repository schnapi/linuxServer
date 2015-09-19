CC = gcc #CC compiler
CFLAGS = -pthread -D_GNU_SOURCE=1 -Wall # arguments for the compiler, -c compile source code to an object file, -o (output file) is not needed if -c is used, -Wall show useful warnings, -D_GNU_SOURCE=1 we shall define due of compiler warnings
DAT = src/server.c src/logger.c src/dateTime.c src/utilityHTTP.c src/utilityManageFiles.c #name of the files we want to compile
TARGET = server #output executable

all: $(DAT)
	gcc $(CFLAGS) src/*.c -c #c files to object files
	gcc $(CFLAGS) *.o -o $(TARGET) #object files to executable
	mv *.o objects #move all object file in object dir
clean: 
	rm -rf $(TARGET)
	
