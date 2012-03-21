#include <gst/gst.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <zmq.h>
#include "zhelpers.h"
#include "utils.h"
#include <poll.h>

#define MAX_CHANNELS 20
#define ARG_SEP ' '

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

void
p_play ( gchar ** args, GSList ** channels, guint n_channels )
{

    printf( "playing %s in channel %s", args[1], args[0] );
    
    //GstElement * pipeline;
    //pipeline = make_pipeline(  );
}

void
p_dispatch ( gchar * request, GSList ** channels, guint n_channels )
{
  // divide message into [fn ..args]
  gchar sep = ARG_SEP;
  gchar ** fn_args = g_strsplit( request, &sep, 1 );

  // split function and args
  gchar * fn = (*fn_args);
  gchar ** args = fn_args + 1;

  if ( ! g_strcmp0 ( fn, "play" ))
  {
      p_play ( args, channels, n_channels );
  }
  else
  {
      g_warning ( "did not understand message: %s", request );
  }
}

struct Control
{
    gpointer socket;
    GSList ** channels;
    guint n_channels;
};

gboolean
p_respond(gpointer data)
{
  // unpack ctrl
  struct Control * ctrl = (struct Control *) data;
  gpointer sock = (*ctrl).socket;
  GSList ** channels = (*ctrl).channels;
  guint n_channels = (*ctrl).n_channels;
  
  //  Wait for next request from client
  gchar * request;
  request = s_recv (sock);

  //  Log raw request
  g_message ( "received message: %s", request );

  //  Dispatch request
  p_dispatch ( request, channels, n_channels );

  //  Send reply back to client
  s_send(sock, request);

  //  Free request
  free(request);

  return TRUE;
}

static gboolean
zmq_fd_prepare(GSource *source,
           gint *timeout)
{
  *timeout = -1;
  return FALSE;
}

static gboolean
zmq_fd_check (GSource *source)
{
  return TRUE;
}

static gboolean
zmq_fd_dispatch(GSource* source, GSourceFunc callback, gpointer user_data)
{
  callback(user_data);
}

static GSourceFuncs 
zmq_source_funcs()
{
  GSourceFuncs r = { zmq_fd_prepare, zmq_fd_check, zmq_fd_dispatch, NULL, NULL, NULL };
  return r;
}

/* make control object */
gpointer
p_make_control ( gpointer sock )
{
  guint n_chans = MAX_CHANNELS;
  
  struct Control * ctrl = malloc ( sizeof ( * ctrl ) );
  (*ctrl).socket = sock;
  (*ctrl).channels = malloc ( n_chans * sizeof ( GList ) );
  (*ctrl).n_channels = n_chans;

  // make channels
  // populate control with empty channels
  gint i = 0;
  for ( i = 0; i < n_chans; i++ )
      (*ctrl).channels[i] = g_slist_alloc ();

  return (gpointer) ctrl;
}

gint zmq_poll_adapter(GPollFD *ufds, guint nfsd, gint timeout_)
{
    zmq_pollitem_t ufds0[nfsd];

    int i;
    for (i = 0; i < nfsd; i++)
    {
        ufds0[i].socket = NULL;
        ufds0[i].fd = ufds[i].fd;
        ufds0[i].events = ufds[i].events;
        ufds0[i].revents = ufds[i].revents;

    }

    zmq_poll(ufds0, nfsd, timeout_);
}

/* main */

gint
main (gint   argc,
      gchar *argv[])
{

  print_version ();

  /* initialize gstreamer */
  gst_init (&argc, &argv);

  /** get main context */
  GMainContext * g_context;
  g_context = g_main_context_new ();

  /** get main loop */
  loop = g_main_loop_new ( g_context, FALSE );

  /* initialize zeromq */

  /** zmq main context */
  gpointer zmq_context = zmq_init (1);

  /** intialize socket to talk to clients */
  gpointer zmq_sock = zmq_socket (zmq_context, ZMQ_REP);
  zmq_bind (zmq_sock, "tcp://*:5555");

  /* make zmq socket into a source for glib */

  /** set poll fn to operate on zmq or unix sockets */
  g_main_context_set_poll_func( g_context, (GPollFunc) zmq_poll_adapter );

  /** make new gsource */
  GSource * g_source;
  GSourceFuncs g_source_funcs;
  g_source_funcs = zmq_source_funcs();
  g_source = g_source_new ( &g_source_funcs, sizeof(GSource) );

  /** add zmq socket to be polled */
  zmq_pollitem_t zmq_item;
  zmq_item.socket = zmq_sock;
  zmq_item.events = ZMQ_POLLIN;
  g_source_add_poll( g_source, (GPollFD *) &zmq_item );

  /** make control to be used by callback */
  gpointer p_control;
  p_control = p_make_control( zmq_sock );    

  /** add callback */
  g_source_set_callback( g_source, (GSourceFunc) p_respond, p_control, NULL );

  g_source_attach( g_source, g_context );


  g_main_loop_run ( loop );

  /* cleanup */

  /** gstreamer */
  //gst_element_set_state (pipeline, GST_STATE_NULL);
  //gst_object_unref (GST_OBJECT (pipeline));

  g_main_loop_unref ( loop );
  g_main_context_unref ( g_context );

  /** zmq */
  zmq_close (zmq_sock);
  zmq_term (zmq_context);

  return 0;
}


//int zmq_poll (void *socket, int option_name, void *option_value, size_t *option_len)
//{
//  return (( ZMQ_POLLING & getsockopt( socket, ZMQ_EVENTS, option_value, option_len )) != 0 );
//}

