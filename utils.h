void
log_version ( level )
{
  const gchar *nano_str;
  guint major, minor, micro, nano;

  // gst version
  gst_version (&major, &minor, &micro, &nano);

  if (nano == 1)
    nano_str = "(CVS)";
  else if (nano == 2)
    nano_str = "(Prerelease)";
  else
    nano_str = ""; 

  g_log ( G_LOG_DOMAIN, level,
          ( "GStreamer version is %d.%d.%d %s" ),
          major, minor, micro, nano_str );


  // zmq version
  zmq_version (&major, &minor, &micro);
  
  g_log ( G_LOG_DOMAIN, level,
          "ØMQ version is %d.%d.%d",
          major, minor, micro );

  return;
}

void
p_null_args ( gchar * fn )
{
    g_warning ( "%s() was given null args" );
}

