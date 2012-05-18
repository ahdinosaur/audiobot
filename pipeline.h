
#include	<glib-object.h>

#ifdef G_ENABLE_DEBUG
#define g_marshal_value_peek_boolean(v)  g_value_get_boolean (v)
#define g_marshal_value_peek_char(v)     g_value_get_schar (v)
#define g_marshal_value_peek_uchar(v)    g_value_get_uchar (v)
#define g_marshal_value_peek_int(v)      g_value_get_int (v)
#define g_marshal_value_peek_uint(v)     g_value_get_uint (v)
#define g_marshal_value_peek_long(v)     g_value_get_long (v)
#define g_marshal_value_peek_ulong(v)    g_value_get_ulong (v)
#define g_marshal_value_peek_int64(v)    g_value_get_int64 (v)
#define g_marshal_value_peek_uint64(v)   g_value_get_uint64 (v)
#define g_marshal_value_peek_enum(v)     g_value_get_enum (v)
#define g_marshal_value_peek_flags(v)    g_value_get_flags (v)
#define g_marshal_value_peek_float(v)    g_value_get_float (v)
#define g_marshal_value_peek_double(v)   g_value_get_double (v)
#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_param(v)    g_value_get_param (v)
#define g_marshal_value_peek_boxed(v)    g_value_get_boxed (v)
#define g_marshal_value_peek_pointer(v)  g_value_get_pointer (v)
#define g_marshal_value_peek_object(v)   g_value_get_object (v)
#define g_marshal_value_peek_variant(v)  g_value_get_variant (v)
#else /* !G_ENABLE_DEBUG */
/* WARNING: This code accesses GValues directly, which is UNSUPPORTED API.
 *          Do not access GValues directly in your code. Instead, use the
 *          g_value_get_*() functions
 */
#define g_marshal_value_peek_boolean(v)  (v)->data[0].v_int
#define g_marshal_value_peek_char(v)     (v)->data[0].v_int
#define g_marshal_value_peek_uchar(v)    (v)->data[0].v_uint
#define g_marshal_value_peek_int(v)      (v)->data[0].v_int
#define g_marshal_value_peek_uint(v)     (v)->data[0].v_uint
#define g_marshal_value_peek_long(v)     (v)->data[0].v_long
#define g_marshal_value_peek_ulong(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_int64(v)    (v)->data[0].v_int64
#define g_marshal_value_peek_uint64(v)   (v)->data[0].v_uint64
#define g_marshal_value_peek_enum(v)     (v)->data[0].v_long
#define g_marshal_value_peek_flags(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_float(v)    (v)->data[0].v_float
#define g_marshal_value_peek_double(v)   (v)->data[0].v_double
#define g_marshal_value_peek_string(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_param(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_boxed(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_pointer(v)  (v)->data[0].v_pointer
#define g_marshal_value_peek_object(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_variant(v)  (v)->data[0].v_pointer
#endif /* !G_ENABLE_DEBUG */


/* VOID:OBJECT,OBJECT,BOOLEAN (/dev/stdin:1) */
void
g_cclosure_marshal_VOID__OBJECT_OBJECT_BOOLEAN (GClosure     *closure,
                                                     GValue       *return_value G_GNUC_UNUSED,
                                                     guint         n_param_values,
                                                     const GValue *param_values,
                                                     gpointer      invocation_hint G_GNUC_UNUSED,
                                                     gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT_BOOLEAN) (gpointer     data1,
                                                            gpointer     arg_1,
                                                            gpointer     arg_2,
                                                            gboolean     arg_3,
                                                            gpointer     data2);
  register GMarshalFunc_VOID__OBJECT_OBJECT_BOOLEAN callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 4);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__OBJECT_OBJECT_BOOLEAN) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_marshal_value_peek_object (param_values + 1),
            g_marshal_value_peek_object (param_values + 2),
            g_marshal_value_peek_boolean (param_values + 3),
            data2);
}


static GMainLoop *loop;

static gboolean
p_bus_callback (GstBus     *bus,
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

static void
p_cb_newpad ( gpointer data1,
        GstElement *decodebin,
        GstPad     *pad,
        gboolean    last,
        gpointer    data2)
{
    GstCaps *caps;
    GstStructure *str;
    GstPad *audiopad;

    GstElement * audio = (GstElement *) data1;
    g_message ( "%s", data1 );
    g_message ( "%s", data2 );

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
p_make_pipeline ( gchar * uri )
{
    // initialize variables
    GstElement *src, *dec, *conv, *sink;
    GstPad *audiopad;
    GstBus *bus;

    GstElement *audio;

    // make pipeline
    GstElement * pipeline;
    pipeline = gst_pipeline_new ( uri );

    // make bus
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, p_bus_callback, loop);
    gst_object_unref (bus);

    // setup closure for decodebin2
    GClosure * cc_p_cb_newpad;
    cc_p_cb_newpad = g_cclosure_new ( G_CALLBACK ( p_cb_newpad ), (gpointer) audio, NULL );
    g_closure_set_marshal ( cc_p_cb_newpad, g_cclosure_marshal_VOID__OBJECT_OBJECT_BOOLEAN );

    // make source
    src = gst_element_make_from_uri(GST_URI_SRC, uri, "source");
    // make decodebin2 decoder
    dec = gst_element_factory_make ("decodebin2", "decoder");
    // decodebin2 generates a new-decoded-pad signal on every new decoded pad
    g_signal_connect (dec, "new-decoded-pad", G_CALLBACK (cc_p_cb_newpad), NULL);
    // add and link
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

    return pipeline;
}


