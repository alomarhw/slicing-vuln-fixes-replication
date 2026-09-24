iwl_sta_alloc_lq(struct iwl_priv *priv, struct iwl_rxon_context *ctx,
		 u8 sta_id)
{
	struct iwl_link_quality_cmd *link_cmd;

	link_cmd = kzalloc(sizeof(struct iwl_link_quality_cmd), GFP_KERNEL);
	if (!link_cmd) {
		IWL_ERR(priv, "Unable to allocate memory for LQ cmd.\n");
		return NULL;
	}

	iwl_sta_fill_lq(priv, ctx, sta_id, link_cmd);

	return link_cmd;
}
