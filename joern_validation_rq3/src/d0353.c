int iwl_send_lq_cmd(struct iwl_priv *priv, struct iwl_rxon_context *ctx,
		    struct iwl_link_quality_cmd *lq, u8 flags, bool init)
{
	int ret = 0;
	unsigned long flags_spin;

	struct iwl_host_cmd cmd = {
		.id = REPLY_TX_LINK_QUALITY_CMD,
		.len = { sizeof(struct iwl_link_quality_cmd), },
		.flags = flags,
		.data = { lq, },
	};

	if (WARN_ON(lq->sta_id == IWL_INVALID_STATION))
		return -EINVAL;


	spin_lock_irqsave(&priv->shrd->sta_lock, flags_spin);
	if (!(priv->stations[lq->sta_id].used & IWL_STA_DRIVER_ACTIVE)) {
		spin_unlock_irqrestore(&priv->shrd->sta_lock, flags_spin);
		return -EINVAL;
	}
	spin_unlock_irqrestore(&priv->shrd->sta_lock, flags_spin);

	iwl_dump_lq_cmd(priv, lq);
	if (WARN_ON(init && (cmd.flags & CMD_ASYNC)))
		return -EINVAL;

	if (is_lq_table_valid(priv, ctx, lq))
		ret = iwl_trans_send_cmd(trans(priv), &cmd);
	else
		ret = -EINVAL;

	if (cmd.flags & CMD_ASYNC)
		return ret;

	if (init) {
		IWL_DEBUG_INFO(priv, "init LQ command complete, "
			       "clearing sta addition status for sta %d\n",
			       lq->sta_id);
		spin_lock_irqsave(&priv->shrd->sta_lock, flags_spin);
		priv->stations[lq->sta_id].used &= ~IWL_STA_UCODE_INPROGRESS;
		spin_unlock_irqrestore(&priv->shrd->sta_lock, flags_spin);
	}
	return ret;
}
