CC = gcc #CC rezervirana beseda za prevajalnike
CFLAGS = -pthread -o # rezervirana beseda za te znake
DAT = src/server.c src/logger.c #ime datoteke
TARGET = server #ime datoteke

all: $(DAT)
	${CC} $(CFLAGS) $(TARGET) $(DAT) #prevedemo datoteko in jo shranimo v iskanje
clean: 
	rm -rf $(TARGET) #izbrise se ko klicemo clean
