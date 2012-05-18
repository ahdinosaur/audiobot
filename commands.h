gchar *
p_play ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];

    if (chan_name == NULL)
        p_null_args ( "play" );

    // play channel
    GstStateChangeReturn change;
    change = p_play_channel_by_name ( channels, chan_name );
    g_debug ( "play(%s) => %d of enum GstStateChangeReturn", chan_name, change );

    if (change)
        return "ok";
    else
        return "fail";
}

gchar *
p_pause ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];

    if (chan_name == NULL)
        p_null_args ( "pause" );

    // pause channel
    GstStateChangeReturn change;
    change = p_pause_channel_by_name ( channels, chan_name );
    g_debug ( "pause(%s) => %d of enum GstStateChangeReturn", chan_name, change );

    if (change)
        return "ok";
    else
        return "fail";
}

gchar *
p_stop ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];

    if (chan_name == NULL)
        p_null_args ( "stop" );

    // stop channel
    GstStateChangeReturn change;
    change = p_stop_channel_by_name ( channels, chan_name );
    g_debug ( "stop(%s) => %d of enum GstStateChangeReturn", chan_name, change );

    if (change)
        return "ok";
    else
        return "fail";
}

gchar *
p_status ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];

    if (chan_name == NULL)
        p_null_args ( "status" );

    // channel status
    GstState status = p_channel_status_by_name ( channels, chan_name );
    g_debug ( "status(%s) => %d of enum GstState", chan_name, status );

    switch ( status )
    {
        case GST_STATE_VOID_PENDING:
            return "pending";
        case GST_STATE_NULL:
            return "empty";
        case GST_STATE_READY:
            return "off";
        case GST_STATE_PAUSED:
            return "paused";
        case GST_STATE_PLAYING:
            return "playing";
        default:
            return "fail";
    }
}

gchar *
p_push ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];
    gchar * uri = args[1];

    g_debug ( "push(%s, %s) called", chan_name, uri );

    if (chan_name == NULL || uri == NULL)
    {
        p_null_args ( "queue" );
        return;
    }

    // get channel
    GQueue * channel = p_get_channel ( channels, chan_name );
    // if channel doesn't exist yet,
    if ( channel == NULL )
    {
        // create new channel
        channel = p_new_channel ();
        // insert the new channel into channels
        p_insert_channel ( channels, chan_name, channel );
    }
    // assert we have a channel and in channels
    assert ( channel != NULL );
    assert ( channel == p_get_channel ( channels, chan_name ) );

    // make musical pipeline from desired uri
    GstElement * pipeline;
    pipeline = p_make_pipeline ( uri );
    if ( pipeline )
    {
      // if channel is already playing,
      if ( p_channel_status ( channel ) == GST_STATE_PLAYING )
      {
          // remove head
          GstElement * head = p_channel_pop_head ( channel );
          gst_object_unref ( GST_OBJECT ( head ) );
          // add new pipeline
          p_channel_push_head ( channel, pipeline );
          // continue playing
          p_play_channel ( channel );
      } else {
          // add new pipeline to channel
          p_channel_push_head ( channel, pipeline );
      }

      g_message ( "pushing %s on channel %s", pipeline, channel );
    }
    else
    {
      g_warning ( "%s could not be pushed", uri );
      return "could not be pushed";
    }

    return "ok";
}
gchar *
p_queue ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];
    gchar * uri = args[1];

    g_debug ( "queue(%s, %s) called", chan_name, uri );

    if (chan_name == NULL || uri == NULL)
    {
        p_null_args ( "queue" );
        return;
    }

    // get channel
    GQueue * channel = p_get_channel ( channels, chan_name );
    // if channel doesn't exist yet,
    if ( channel == NULL )
    {
        // create new channel
        channel = p_new_channel ();
        // insert the new channel into channels
        p_insert_channel ( channels, chan_name, channel );
    }
    // assert we have a channel and in channels
    assert ( channel != NULL );
    assert ( channel == p_get_channel ( channels, chan_name ) );

    // make musical pipeline from desired uri
    GstElement * pipeline;
    pipeline = p_make_pipeline ( uri );
    if ( pipeline )
    {
      // add new pipeline to channel
      g_message ( "queueing %s on channel %s", uri, chan_name );
      p_channel_push_tail ( channel, pipeline );
    }
    else
    {
      g_warning ( "%s could not be queued", uri );
      return "could not be queued";
    }

    return "ok";
}


gchar *
p_skip ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];

    if (chan_name == NULL)
        p_null_args ( "next" );

    // pop head
    GstElement * head = p_channel_pop_head_by_name ( channels, chan_name );
    gst_object_unref ( GST_OBJECT ( head ) );
    // play remaining pipelines
    GstStateChangeReturn change;
    change = p_play_channel_by_name ( channels, chan_name );
    g_debug ( "skip(%s) => %d of enum GstStateChangeReturn", chan_name, change );

    if (change)
        return "ok";
    else
        return "fail";
}


gchar *
p_length ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];

    if (chan_name == NULL)
        p_null_args ( "length" );


    guint len = p_channel_length_by_name ( channels, chan_name );
    gchar * len_s = ( gchar * ) malloc ( 60 * sizeof(gchar) );
    g_snprintf ( len_s, 60, "%d", len );
    return len_s;
}

gchar *
p_now_playing ( gchar ** args, GHashTable * channels )
{
    gchar * chan_name = args[0];

    if (chan_name == NULL)
        p_null_args ( "length" );

    GstElement * head = p_channel_peek_head_by_name ( channels, chan_name  );
    return gst_element_get_name ( head );
}
