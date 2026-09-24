static BOOL update_send_set_keyboard_indicators(rdpContext* context,
        UINT16 led_flags)
{
	wStream* s;
	rdpRdp* rdp = context->rdp;
	s = rdp_data_pdu_init(rdp);

	if (!s)
		return FALSE;

	Stream_Write_UINT16(s,
	                    0); /* unitId should be 0 according to MS-RDPBCGR 2.2.8.2.1.1 */
	Stream_Write_UINT16(s, led_flags); /* ledFlags (2 bytes) */
	return rdp_send_data_pdu(rdp, s, DATA_PDU_TYPE_SET_KEYBOARD_INDICATORS,
	                         rdp->mcs->userId);
}
