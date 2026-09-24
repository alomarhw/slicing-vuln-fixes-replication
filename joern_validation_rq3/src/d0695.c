qtdemux_parse_tree (GstQTDemux * qtdemux)
{
  GNode *mvhd;
  GNode *trak;
  GNode *udta;
  gint64 duration;

  mvhd = qtdemux_tree_get_child_by_type (qtdemux->moov_node, FOURCC_mvhd);
  if (mvhd == NULL) {
    GST_LOG_OBJECT (qtdemux, "No mvhd node found, looking for redirects.");
    return qtdemux_parse_redirects (qtdemux);
  }

  qtdemux->timescale = QT_UINT32 ((guint8 *) mvhd->data + 20);
  qtdemux->duration = QT_UINT32 ((guint8 *) mvhd->data + 24);

  GST_INFO_OBJECT (qtdemux, "timescale: %u", qtdemux->timescale);
  GST_INFO_OBJECT (qtdemux, "duration: %u", qtdemux->duration);

  /* set duration in the segment info */
  gst_qtdemux_get_duration (qtdemux, &duration);
  gst_segment_set_duration (&qtdemux->segment, GST_FORMAT_TIME, duration);

  /* parse all traks */
  trak = qtdemux_tree_get_child_by_type (qtdemux->moov_node, FOURCC_trak);
  while (trak) {
    qtdemux_parse_trak (qtdemux, trak);
    /* iterate all siblings */
    trak = qtdemux_tree_get_sibling_by_type (trak, FOURCC_trak);
  }
  gst_element_no_more_pads (GST_ELEMENT_CAST (qtdemux));

  /* find and push tags, we do this after adding the pads so we can push the
   * tags downstream as well. */
  udta = qtdemux_tree_get_child_by_type (qtdemux->moov_node, FOURCC_udta);
  if (udta) {
    qtdemux_parse_udta (qtdemux, udta);

    if (qtdemux->tag_list) {
      GST_DEBUG_OBJECT (qtdemux,
          "calling gst_element_found_tags with %" GST_PTR_FORMAT,
          qtdemux->tag_list);
      gst_element_found_tags (GST_ELEMENT_CAST (qtdemux), qtdemux->tag_list);
      qtdemux->tag_list = NULL;
    }
  } else {
    GST_LOG_OBJECT (qtdemux, "No udta node found.");
  }
  return TRUE;
}
