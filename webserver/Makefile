CC = gcc #CC compiler
CFLAGS = -pthread -o # arguments for the compiler
DAT = src/server.c src/logger.c src/dateTime.c src/utilityHTTP.c #name of the files we want to compile
TARGET = server #output executable

all: $(DAT)
	${CC} $(CFLAGS) $(TARGET) $(DAT)
clean: 
	rm -rf $(TARGET)
	
