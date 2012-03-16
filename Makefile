C=gcc 
LIBS=`pkg-config gstreamer-0.10 --libs` -lzmq
CFLAGS=`pkg-config gstreamer-0.10 --cflags`

all: server.o client.o

server.o:
	$(CC) $(LIBS) $(CFLAGS) -o server server.c

client.o:
	$(CC) $(LIBS) $(CFLAGS) -o client client.c
