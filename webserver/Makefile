CC = gcc #CC compiler
CFLAGS = -pthread -D_GNU_SOURCE=1 -Wall # arguments for the compiler, -Wall show useful warnings, -D_GNU_SOURCE=1 we shall define due of compiler warnings for asprintf function
#DAT = src/server.c src/logger.c src/dateTime.c src/utilityHTTP.c src/utilityManageFiles.c #name of the files we want to compile
TARGET = server #output executable

all: #$(DAT)
	gcc $(CFLAGS) src/*.c -c #-c compile source code to an object file
	gcc $(CFLAGS) *.o -o $(TARGET) #-o object files to executable
	mv *.o objects #move all object file in object dir
clean: 
	rm -rf $(TARGET) #remove executable file
	rm -rf objects/*.o #remove objectives files
	
