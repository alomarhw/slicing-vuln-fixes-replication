static void btif_hh_handle_evt(UINT16 event, char *p_param)
{
 bt_bdaddr_t *bd_addr = (bt_bdaddr_t*)p_param;
    BTIF_TRACE_EVENT("%s: event=%d", __FUNCTION__, event);
 int ret;
 switch(event)
 {
 case BTIF_HH_CONNECT_REQ_EVT:
 {
            ret = btif_hh_connect(bd_addr);
 if(ret == BT_STATUS_SUCCESS)
 {
                HAL_CBACK(bt_hh_callbacks, connection_state_cb,bd_addr,BTHH_CONN_STATE_CONNECTING);
 }
 else
                HAL_CBACK(bt_hh_callbacks, connection_state_cb,bd_addr,BTHH_CONN_STATE_DISCONNECTED);
 }
 break;

 case BTIF_HH_DISCONNECT_REQ_EVT:
 {
            BTIF_TRACE_EVENT("%s: event=%d", __FUNCTION__, event);
            btif_hh_disconnect(bd_addr);
            HAL_CBACK(bt_hh_callbacks, connection_state_cb,bd_addr,BTHH_CONN_STATE_DISCONNECTING);
 }
 break;

 case BTIF_HH_VUP_REQ_EVT:
 {
            BTIF_TRACE_EVENT("%s: event=%d", __FUNCTION__, event);
            ret = btif_hh_virtual_unplug(bd_addr);
 }
 break;

 default:
 {
            BTIF_TRACE_WARNING("%s : Unknown event 0x%x", __FUNCTION__, event);
 }
 break;
 }
}
