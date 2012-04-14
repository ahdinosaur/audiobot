#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <gst/gst.h>
#include <zmq.h>
#include <czmq.h>

#include "control.h"
#include "pipeline.h"
#include "channel.h"
#include "utils.h"

#define MAX_CHANNELS 20
#define ARG_DELIMS " "

void
p_play ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];
    gchar * uri = args[1];

    g_debug ( "play(%s, %s) called", chan_name, uri );

    if (chan_name == NULL || uri == NULL)
    {
        p_null_args ( "play" );
        return;
    }

    // get channel
    GQueue * channel = p_get_channel ( channels, chan_name );
    // if channel doesn't exist yet,
    if ( channel == NULL )
    {
        // create new channel
        channel = p_new_channel ();
    }
    // assert we have a channel
    assert ( channel != NULL );

    // make musical pipeline from desired uri
    GstElement * pipeline;
    pipeline = p_make_pipeline ( uri );
    if ( pipeline )
    {
      g_message ("now playing: %s\n", uri);

      /* add pipeline to the desired channel */
      g_queue_push_head ( channel, (gpointer) pipeline );

      /* play pipeline */
      gst_element_set_state (pipeline, GST_STATE_PLAYING );
    }
    else
    {
      g_warning ("%s could not be played", uri);
    }

    return;
}



void
p_stop ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];

    g_debug ( "stop(%s) called", chan_name );
    
    if (chan_name == NULL)
        p_null_args ( "stop" );

}


/* Given a request, returns a response */
zmsg_t *
p_respond ( zmsg_t * request, GHashTable * channels )
{
  // Split request into fn ...args
  gchar * fn;
  fn = zmsg_popstr ( request );

  size_t n_args = zmsg_size (request);
  gchar ** args = malloc ( (n_args + 1) * sizeof ( gchar * ));
  gint i;
  for ( i = 0; i < n_args; i++ )
  {
      args[i] = zmsg_popstr ( request );
  }

  g_debug ("fn=[%s]", fn);

  if ( ! g_strcmp0 ( fn, "play" ))
  {
      p_play ( args, channels );
  }
  else
  {
      g_warning ( "undefined function: %s", fn );
  }
}



gboolean
p_cycle(gpointer data)
{
  // unpack ctrl
  struct Control * ctrl = (struct Control *) data;
  gpointer sock = (*ctrl).socket;
  GHashTable * channels = (*ctrl).channels;
  
  //  Wait for next request from client
  zmsg_t * request;
  request = zmsg_recv ( sock );

  //  Log raw request

  //  Send request and get response
  zmsg_t * response;
  response = p_respond ( request, channels );

  //  Send reply back to client
  zmsg_send ( sock, request );

  //  Destroy request
//  zmsg_destroy ( &request );

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
  struct Control * ctrl = malloc ( sizeof ( * ctrl ) );
  (*ctrl).socket = sock;
  //TODO write destroy funcs for key,value
  // http://developer.gnome.org/glib/unstable/glib-Hash-Tables.html#g-hash-table-new-full
  (*ctrl).channels = g_hash_table_new_full ( g_str_hash, g_str_equal, NULL, NULL );

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
  g_source_set_callback( g_source, (GSourceFunc) p_cycle, p_control, NULL );

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

