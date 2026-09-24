SpoolssEnumPrinterData_q(tvbuff_t *tvb, int offset,
				    packet_info *pinfo, proto_tree *tree,
				    dcerpc_info *di, guint8 *drep _U_)
{
	guint32 ndx;
	proto_item *hidden_item;

	hidden_item = proto_tree_add_uint(
		tree, hf_printerdata, tvb, offset, 0, 1);
	PROTO_ITEM_SET_HIDDEN(hidden_item);

	/* Parse packet */

	offset = dissect_nt_policy_hnd(
		tvb, offset, pinfo, tree, di, drep, hf_hnd, NULL, NULL,
		FALSE, FALSE);

	offset = dissect_ndr_uint32(
		tvb, offset, pinfo, tree, di, drep,
		hf_enumprinterdata_enumindex, &ndx);

	col_append_fstr(pinfo->cinfo, COL_INFO, ", index %d", ndx);

	offset = dissect_ndr_uint32(
		tvb, offset, pinfo, tree, di, drep,
		hf_enumprinterdata_value_offered, NULL);

	offset = dissect_ndr_uint32(
		tvb, offset, pinfo, tree, di, drep,
		hf_enumprinterdata_data_offered, NULL);

	return offset;
}
