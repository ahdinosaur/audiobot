#include <gst/gst.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zmq.h>
#include "zhelpers.h"
#include "utils.h"

/* gstreamer-related functions */

static GMainLoop *loop;

static gboolean
my_bus_callback (GstBus     *bus,
         GstMessage *message,
         gpointer    data)
{
  //g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

  switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR: {
      GError *err;
      gchar *debug;

      gst_message_parse_error (message, &err, &debug);
      g_print ("Error: %s\n", err->message);
      g_error_free (err);
      g_free (debug);

      g_main_loop_quit (loop);
      break;
    }
    case GST_MESSAGE_EOS:
      /* end-of-stream */
      g_main_loop_quit (loop);
      break;
    default:
      /* unhandled message */
      break;
  }

  /* we want to be notified again the next time there is a message
   * on the bus, so returning TRUE (FALSE means we want to stop watching
   * for messages on the bus and our callback should not be called again)
   */
  return TRUE;
}
/* TODO: de-global-ize */
GstElement *audio;

static void
cb_newpad (GstElement *decodebin,
       GstPad     *pad,
       gboolean    last,
       gpointer    data)
{
  GstCaps *caps;
  GstStructure *str;
  GstPad *audiopad;

  /* only link once */
  audiopad = gst_element_get_static_pad (audio, "sink");
  if (GST_PAD_IS_LINKED (audiopad)) {
    g_object_unref (audiopad);
    return;
  }

  /* check media type */
  caps = gst_pad_get_caps (pad);
  str = gst_caps_get_structure (caps, 0);
  if (!g_strrstr (gst_structure_get_name (str), "audio")) {
    gst_caps_unref (caps);
    gst_object_unref (audiopad);
    return;
  }
  gst_caps_unref (caps);

  /* link'n'play */
  gst_pad_link (pad, audiopad);

  g_object_unref (audiopad);
}


GstElement *
make_pipeline ( gchar * uri )
{
  /* initialize variables */
  GstElement *src, *dec, *conv, *sink;
  GstPad *audiopad;
  GstBus *bus;

  /* setup */
  GstElement * pipeline;
  pipeline = gst_pipeline_new ("pipeline");

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, my_bus_callback, loop);
  gst_object_unref (bus);

  s_console ("now playing: %s\n", uri);
  src = gst_element_make_from_uri(GST_URI_SRC, uri, "source");
  dec = gst_element_factory_make ("decodebin", "decoder");
  g_signal_connect (dec, "new-decoded-pad", G_CALLBACK (cb_newpad), NULL);
  gst_bin_add_many (GST_BIN (pipeline), src, dec, NULL);
  gst_element_link (src, dec);

  /* create audio output */
  audio = gst_bin_new ("audiobin");
  conv = gst_element_factory_make ("audioconvert", "aconv");
  audiopad = gst_element_get_static_pad (conv, "sink");
  sink = gst_element_factory_make ("alsasink", "sink");
  gst_bin_add_many (GST_BIN (audio), conv, sink, NULL);
  gst_element_link (conv, sink);
  gst_element_add_pad (audio,
      gst_ghost_pad_new ("sink", audiopad));
  gst_object_unref (audiopad);
  gst_bin_add (GST_BIN (pipeline), audio);

  /* run */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  
  return pipeline;
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

/* main */

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
  zmq_pollitem_t zmq_item[1];
  zmq_item[0].socket = zmq_sock;
  zmq_item[0].events = ZMQ_POLLIN;

  g_source_add_poll( g_source, (GPollFD *) zmq_item );

  /* add callback */
  zmq_callback = 

  g_source_attach( g_source, g_context );

  // GstElement * pipeline
  GstElement * pipeline;
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

