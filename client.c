#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <zmq.h>
#include <czmq.h>

#define MAX_MESSAGE 1024

void sigint_handler ( int signal )
{
    exit(1);
}

char *
prompt_user( char * prompt )
{
    /* print fn prompt for user */
    printf( "%s", prompt );
    /* get fn input from user */
    char input[MAX_MESSAGE];
    fgets (input, MAX_MESSAGE, stdin);
    int input_len = strlen ( input );

    char * s = malloc( input_len * sizeof(char) );
    strncpy ( s, input, input_len - 1);
    s[input_len - 1] = '\0';
    return s;
}

zmsg_t *
prompt_user_for_msg ()
{
    int success;
    // make new message
    zmsg_t * msg = zmsg_new ();
    // first frame is fn
    char * fn = prompt_user ( "fn: " );
    zmsg_addstr ( msg, "%s", fn );

    // subsequent frames are args
    int arg_num = 0;
    while (1)
    {
        char arg_prompt[64];
        sprintf ( arg_prompt, "arg %d: ", arg_num );
        char * arg = prompt_user ( arg_prompt );

        printf("[%s]", arg);

        if ( arg == NULL || *arg == '\0' )
        {
            return msg;
        }
        else
        {
            zmsg_addstr ( msg, "%s", arg );
        }

        arg_num++;
    }
}


int
test ( zmsg_t * request, zmsg_t * expected_response)
{
    
}

int
main ( void )
{
    signal (SIGINT, sigint_handler );

    char *server_uri = "tcp://localhost:5555";

    zctx_t * context = zctx_new ();

    //  connect to socket to talk to server
    printf ( "connecting to %s\n", server_uri );
    void *sock = zsocket_new ( context, ZMQ_REQ );
    zsocket_connect ( sock, server_uri );

    // request - reply loop
    while (1)
    {
        // prompt for request
        zmsg_t * request;
        request = prompt_user_for_msg ();
        // send request
        zmsg_send ( &request, sock );
        // receive reply
        zmsg_t * reply;
        reply = zmsg_recv ( sock );
        // process reply
        printf ( "response: %s\n", reply );
        // destroy request - reply
        zmsg_destroy ( &request );
        zmsg_destroy ( &reply );
    }
    // cleanup
    zsocket_destroy ( context, sock );
    zctx_destroy ( &context );
    return 0;
}
