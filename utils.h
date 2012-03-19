void print_version ()
{
  const gchar *nano_str;
  guint major, minor, micro, nano;

  gst_version (&major, &minor, &micro, &nano);

  if (nano == 1)
    nano_str = "(CVS)";
  else if (nano == 2)
    nano_str = "(Prerelease)";
  else
    nano_str = ""; 

  printf ("version: GStreamer %d.%d.%d %s!\n",
          major, minor, micro, nano_str);

  return;
}

