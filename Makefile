C=gcc
LIBS=`pkg-config gstreamer-0.10 --libs`
CFLAGS=`pkg-config gstreamer-0.10 --cflags`

all:
	$(CC) $(LIBS) $(CFLAGS) server.c
