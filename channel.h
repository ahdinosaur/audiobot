/* get channels */

GQueue *
p_get_channel ( GHashTable * channels, gchar * chan_name )
{
    return (GQueue *) g_hash_table_lookup ( channels, chan_name );
}

void
p_insert_channel ( GHashTable * channels, gchar * chan_name, GQueue * channel )
{
    g_hash_table_insert ( channels, chan_name, channel );
}

GQueue *
p_new_channel ()
{
    return g_queue_new ();
}

guint
p_channel_length ( GQueue * channel )
{
    return g_queue_get_length ( channel );
}

p_channel_length_by_name ( GHashTable * channels, gchar * chan_name )
{
    return p_channel_length ( p_get_channel ( channels, chan_name ) );
}

/* channel head */

void
p_channel_push_head ( GQueue * channel, GstElement * pipeline )
{
    g_debug ( "adding pipeline %p to channel %p head", pipeline, channel );
    g_queue_push_head ( channel, (gpointer) pipeline );
}


GstElement *
p_channel_pop_head ( GQueue * channel )
{
    GstElement * head = (GstElement *) g_queue_pop_head ( channel );
    gst_element_set_state ( head, GST_STATE_NULL );
    return head;
}

GstElement *
p_channel_pop_head_by_name ( GHashTable * channels, gchar * chan_name )
{
    return p_channel_pop_head ( p_get_channel ( channels, chan_name ) );
}


GstElement *
p_channel_peek_head ( GQueue * channel )
{
    return (GstElement *) g_queue_peek_head ( channel );
}

GstElement *
p_channel_peek_head_by_name ( GHashTable * channels, gchar * chan_name )
{
    return p_channel_peek_head ( p_get_channel ( channels, chan_name ) );
}


/* channel tail */
void
p_channel_push_tail ( GQueue * channel, GstElement * pipeline )
{
    g_debug ( "adding pipeline %p to channel %p tail", pipeline, channel );
    g_queue_push_tail ( channel, pipeline );
}

/* play, pause, stop, status channel */

GstStateChangeReturn
p_play_channel ( GQueue * channel )
{
    GstElement * head = p_channel_peek_head ( channel );
    if (head)
    {
        g_debug ("playing %p in channel %p", head, channel );
        return gst_element_set_state ( head, GST_STATE_PLAYING );
    }
    return 0;
}

GstStateChangeReturn
p_play_channel_by_name ( GHashTable * channels, gchar * chan_name )
{
    g_debug ( "playing channel %s", chan_name);
    return p_play_channel ( p_get_channel ( channels, chan_name ) );
}


GstStateChangeReturn
p_pause_channel ( GQueue * channel )
{
    GstElement * head = p_channel_peek_head ( channel );
    if (head)
    {
        g_debug ("pausing %p in channel %p", head, channel );
        return gst_element_set_state ( head, GST_STATE_PAUSED );
    }
    return 0;
}

GstStateChangeReturn
p_pause_channel_by_name ( GHashTable * channels, gchar * chan_name )
{
    g_debug ( "pausing channel %s", chan_name);
    return p_pause_channel ( p_get_channel ( channels, chan_name ) );
}

GstStateChangeReturn
p_stop_channel ( GQueue * channel )
{
    GstElement * head = p_channel_peek_head ( channel );
    if (head)
    {
        g_debug ("stopping %p in channel %p", head, channel );
        return gst_element_set_state ( head, GST_STATE_NULL );
    }
    return 0;
}

GstStateChangeReturn
p_stop_channel_by_name ( GHashTable * channels, gchar * chan_name )
{
    g_debug ( "stopping channel %s", chan_name );
    return p_stop_channel ( p_get_channel ( channels, chan_name ) );
}

GstState
p_channel_status ( GQueue * channel )
{
    GstElement * head = p_channel_peek_head ( channel );
    if (head)
    {
        GstState state;
        GstStateChangeReturn status;
        status = gst_element_get_state ( head, &state, NULL, GST_CLOCK_TIME_NONE );
        
        g_debug ( "state(%p) => %d of enum GstStateChangeReturn", channel, status );

        return state;
    } else {
        return -1;
    }
}

GstState
p_channel_status_by_name ( GHashTable * channels, gchar * chan_name )
{
    return p_channel_status ( p_get_channel ( channels, chan_name ) );
}

