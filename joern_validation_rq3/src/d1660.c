void smp_send_ltk_reply(tSMP_CB* p_cb, tSMP_INT_DATA* p_data) {
  SMP_TRACE_DEBUG("%s", __func__);
 /* send stk as LTK response */
  btm_ble_ltk_request_reply(p_cb->pairing_bda, true, p_data->key.p_data);
}
