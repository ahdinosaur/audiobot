
GQueue *
p_get_channel ( GHashTable * channels, gchar * chan_name )
{
    return (GQueue *) g_hash_table_lookup ( channels, chan_name );
}

GQueue *
p_new_channel ()
{
    return g_queue_new ();
}
