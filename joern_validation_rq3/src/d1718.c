gst_tag_from_vorbis_tag (const gchar * vorbis_tag)
{
  int i = 0;
  gchar *real_vorbis_tag;

  g_return_val_if_fail (vorbis_tag != NULL, NULL);

  gst_tag_register_musicbrainz_tags ();

  real_vorbis_tag = g_ascii_strup (vorbis_tag, -1);
  while (tag_matches[i].gstreamer_tag != NULL) {
    if (strcmp (real_vorbis_tag, tag_matches[i].original_tag) == 0) {
      break;
    }
    i++;
  }
  g_free (real_vorbis_tag);
  return tag_matches[i].gstreamer_tag;
}
