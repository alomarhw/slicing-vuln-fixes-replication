static void tg3_poll_link(struct tg3 *tp)
{
	/* handle link change and other phy events */
	if (!(tg3_flag(tp, USE_LINKCHG_REG) || tg3_flag(tp, POLL_SERDES))) {
		struct tg3_hw_status *sblk = tp->napi[0].hw_status;

		if (sblk->status & SD_STATUS_LINK_CHG) {
			sblk->status = SD_STATUS_UPDATED |
				       (sblk->status & ~SD_STATUS_LINK_CHG);
			spin_lock(&tp->lock);
			if (tg3_flag(tp, USE_PHYLIB)) {
				tw32_f(MAC_STATUS,
				     (MAC_STATUS_SYNC_CHANGED |
				      MAC_STATUS_CFG_CHANGED |
				      MAC_STATUS_MI_COMPLETION |
				      MAC_STATUS_LNKSTATE_CHANGED));
				udelay(40);
			} else
				tg3_setup_phy(tp, 0);
			spin_unlock(&tp->lock);
		}
	}
}
