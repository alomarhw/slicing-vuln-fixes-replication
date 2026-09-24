gst_asf_demux_change_state (GstElement * element, GstStateChange transition)
{
  GstASFDemux *demux = GST_ASF_DEMUX (element);
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:{
      gst_segment_init (&demux->segment, GST_FORMAT_TIME);
      demux->need_newsegment = TRUE;
      demux->segment_running = FALSE;
      demux->keyunit_sync = FALSE;
      demux->accurate = FALSE;
      demux->adapter = gst_adapter_new ();
      demux->metadata = gst_caps_new_empty ();
      demux->global_metadata = gst_structure_new_empty ("metadata");
      demux->data_size = 0;
      demux->data_offset = 0;
      demux->index_offset = 0;
      demux->base_offset = 0;
      demux->flowcombiner = gst_flow_combiner_new ();

      break;
    }
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_READY:
      gst_asf_demux_reset (demux, FALSE);
      break;

    case GST_STATE_CHANGE_READY_TO_NULL:
      gst_asf_demux_reset (demux, FALSE);
      gst_flow_combiner_free (demux->flowcombiner);
      demux->flowcombiner = NULL;
      break;
    default:
      break;
  }

  return ret;
}
