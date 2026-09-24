atusb_set_promiscuous_mode(struct ieee802154_hw *hw, const bool on)
{
	struct atusb *atusb = hw->priv;
	int ret;

	if (on) {
		ret = atusb_write_subreg(atusb, SR_AACK_DIS_ACK, 1);
		if (ret < 0)
			return ret;

		ret = atusb_write_subreg(atusb, SR_AACK_PROM_MODE, 1);
		if (ret < 0)
			return ret;
	} else {
		ret = atusb_write_subreg(atusb, SR_AACK_PROM_MODE, 0);
		if (ret < 0)
			return ret;

		ret = atusb_write_subreg(atusb, SR_AACK_DIS_ACK, 0);
		if (ret < 0)
			return ret;
	}

	return 0;
}
