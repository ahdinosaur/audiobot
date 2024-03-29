#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <zmq.h>
#include <czmq.h>

#define INIT_MESSAGE_LEN 256

void sigint_handler(int sig)
{
    printf("received signint");
    signal(SIGINT, SIG_DFL);
    kill(getpid(), SIGINT);
}

char *
prompt_user( char * prompt )
{
    // print prompt for user */
    printf( "%s", prompt );
    // get input from user */
    size_t input_n = sizeof(char) * INIT_MESSAGE_LEN;
    char * input = malloc ( input_n );
    if (!getline (&input, &input_n, stdin))
        exit(1);
    int input_len = strlen(input);
    if (input[input_len - 1] == 127
            || input[input_len - 1] == 3
            || input[input_len - 1] == 0)
        exit(1);
    // overwrite newline character
    input[input_len - 1] = '\0';
    rewind(stdin);
    return input;
}

zmsg_t *
prompt_user_for_msg ()
{
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
    signal(SIGINT, sigint_handler);

    char * uri_to_server = "tcp://localhost:5555";

    zctx_t * context = zctx_new ();

    //  connect to socket to talk to server
    printf ( "connecting to %s\n", uri_to_server );
    void * sock = zsocket_new ( context, ZMQ_REQ );
    zsocket_connect ( sock, uri_to_server );

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
