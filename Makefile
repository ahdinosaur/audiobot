C=gcc
LIBS=`pkg-config gstreamer-0.10 --libs` -lzmq
CFLAGS=`pkg-config gstreamer-0.10 --cflags`

all: server.o client.o test.o

server.o:
	$(CC) --debug $(LIBS) $(CFLAGS) -o server server.c

client.o:
	$(CC) --debug $(LIBS) $(CFLAGS) -o client client.c

test.o:
	$(CC) --debug $(LIBS) $(CFLAGS) -o test test.c
