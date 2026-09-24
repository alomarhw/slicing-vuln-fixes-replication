bool btif_av_peer_supports_3mbps(void) {
 bool is3mbps = ((btif_av_cb.edr & BTA_AV_EDR_3MBPS) != 0);
  BTIF_TRACE_DEBUG("%s: connected %d, edr_3mbps %d", __func__,
                   btif_av_is_connected(), is3mbps);
 return (btif_av_is_connected() && is3mbps);
}
