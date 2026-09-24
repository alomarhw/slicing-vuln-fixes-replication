get_file_flags (const char *filename, int *dt)
{
  logprintf (LOG_VERBOSE, _("\
File %s already there; not retrieving.\n\n"), quote (filename));
  /* If the file is there, we suppose it's retrieved OK.  */
  *dt |= RETROKF;

  /* #### Bogusness alert.  */
  /* If its suffix is "html" or "htm" or similar, assume text/html.  */
  if (has_html_suffix_p (filename))
    *dt |= TEXTHTML;
}
