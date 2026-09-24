SpoolssRFNPCNEX_r(tvbuff_t *tvb, int offset, packet_info *pinfo,
			     proto_tree *tree, dcerpc_info *di, guint8 *drep)
{
	/* Parse packet */

	offset = dissect_ndr_pointer(
		tvb, offset, pinfo, tree, di, drep,
		dissect_NOTIFY_INFO, NDR_POINTER_UNIQUE,
		"Notify Info", -1);

	offset = dissect_doserror(
		tvb, offset, pinfo, tree, di, drep, hf_rc, NULL);

	return offset;
}
