#include <stdio.h>
#include <gst/gst.h>

static GMainLoop *loop;

static gboolean
my_bus_callback (GstBus     *bus,
         GstMessage *message,
         gpointer    data)
{
  g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

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

GstElement *pipeline, *audio;

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

gint
main (gint   argc,
      gchar *argv[])
{
  GstElement *src, *dec, *conv, *sink;
  GstPad *audiopad;
  GstBus *bus;

  /* init GStreamer */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  const gchar *nano_str;
  guint major, minor, micro, nano;

  gst_init (&argc, &argv);

  gst_version (&major, &minor, &micro, &nano);

  if (nano == 1)
    nano_str = "(CVS)";
  else if (nano == 2)
    nano_str = "(Prerelease)";
  else
    nano_str = "";

  printf ("Now using GStreamer %d.%d.%d %s!\n",
          major, minor, micro, nano_str);

  /* make sure we have input */
  if (argc != 2) {
    g_print ("Usage: %s <uri>\n", argv[0]);
    return -1;
  }

  /* setup */
  pipeline = gst_pipeline_new ("pipeline");

  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, my_bus_callback, loop);
  gst_object_unref (bus);

  src = gst_element_make_from_uri(GST_URI_SRC, argv[1], "source");
  //g_object_set (G_OBJECT (src), "uri", argv[1], NULL);
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
  g_main_loop_run (loop);

  /* cleanup */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}
