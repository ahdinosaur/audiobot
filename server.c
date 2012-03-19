#include <gst/gst.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zmq.h>
#include "zhelpers.h"
#include "channels.h"
#include "utils.h"

static GMainLoop *loop;

gint
main (gint   argc,
      gchar *argv[])
{

  print_version ();

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* get main context */
  GMainContext * g_context;
  g_context = g_main_context_new();

  /* run */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* initialize zeromq */

  /** zmq main context */
  void *zmq_context = zmq_init (1);

  //  Socket to talk to clients
  void *zmq_sock = zmq_socket (zmq_context, ZMQ_REP);
  zmq_bind (zmq_sock, "tcp://*:5555");


  /* set poll fn to operate on zmq or unix sockets */
  g_main_context_set_poll_func( g_context, (GPollFunc) zmq_poll );

  /* make new gsource */
  GSource * g_source;
  g_source = g_timeout_source_new (1);

  /* add zmq socket to be polled */
  zmq_pollitem_t * zmq_item;
  *(zmq_item).socket = zmq_sock;
  *(zmq_item).events = ZMQ_POLLIN;

  g_source_add_poll( g_source, (GPollFD *) zmq_item );

  g_source_attach( g_source, g_context );

  // GstElement * pipeline;
  pipeline = make_pipeline( argv[1] );

  g_main_loop_run ( loop );

  /* cleanup */

  /** gstreamer */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  /** zmq */
  zmq_close (zmq_sock);
  zmq_term (zmq_context);

  return 0;
}

/* zmq-related functions */
void reqres(void * zmqsock)
{
  //  Wait for next request from client
  char * request;
  request = s_recv (zmqsock);

  //  Do some 'work'
  s_console("message: %s", request);

  //  Send reply back to client
  s_send(zmqsock, request);

  //  Free request
  free(request);

  return;
}

//int zmq_poll (void *socket, int option_name, void *option_value, size_t *option_len)
//{
//  return (( ZMQ_POLLING & getsockopt( socket, ZMQ_EVENTS, option_value, option_len )) != 0 );
//}

//GSource
//zmq_source()
//{
//  
//}

//static gboolean
//zmq_fd_prepare(GSource *source,
//           gint *timeout)
//{
//  *timeout = -1;
//  return FALSE;
//}
//
//static gboolean
//zmq_fd_check (GSource *source)
//{
//  return TRUE;
//}
//
//static gboolean
//zmq_fd_dispatch(GSource* source, GSourceFunc callback, gpointer user_data)
//{
//  static gint counter = 0;
//
//  Display *dpy = ((x11_source_t*)source)->dpy;
//  Window window = ((x11_source_t*)source)->w;
//}

