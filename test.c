#include <gst/gst.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zmq.h>

/* main */

gint
main (gint   argc,
      gchar *argv[])
{

  /* init GStreamer */
  gst_init (&argc, &argv);

  /** get main context */
  GMainContext * g_context;
  g_context = g_main_context_new ();

  /** get main loop */
  GMainLoop * loop;
  loop = g_main_loop_new ( g_context, FALSE );

  /* initialize zeromq */

  /** zmq main context */
  gpointer zmq_context = zmq_init (1);

  /** intialize socket to talk to clients */
  gpointer zmq_sock = zmq_socket (zmq_context, ZMQ_REP);
  zmq_bind (zmq_sock, "tcp://*:5555");

  /* make zmq socket into a source for glib */

  /** set poll fn to operate on zmq or unix sockets */
  g_main_context_set_poll_func( g_context, (GPollFunc) zmq_poll );

  /* .... */

  /* run */
  g_main_loop_run ( loop );

  /* cleanup */

  /** gstreamer */
  g_main_loop_unref (loop);
  g_main_context_unref (g_context);

  /** zmq */
  zmq_close (zmq_sock);
  zmq_term (zmq_context);

  return 0;
}
