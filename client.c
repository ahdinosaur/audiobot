#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <zmq.h>
#include "zhelpers.h"

#define MAX_MESSAGE 100

int main (void)
{
    char *server_uri = "tcp://localhost:5555";

    void *context = zmq_init (1);

    //  Socket to talk to server
    printf ("connecting to %s\n", server_uri);
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, server_uri);

    int request_nbr;
    char input[MAX_MESSAGE];
    char * reply;
    while (1) {
        
        /* get input from user */
        fgets (input, MAX_MESSAGE, stdin);

        s_send(requester, input);

        reply = s_recv (requester);
        printf("%s", reply);
        free(reply);
    }
    zmq_close (requester);
    zmq_term (context);
    return 0;
}
