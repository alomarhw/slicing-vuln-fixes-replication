static int brcmf_cfg80211_set_pmk(struct wiphy *wiphy, struct net_device *dev,
				  const struct cfg80211_pmk_conf *conf)
{
	struct brcmf_if *ifp;

	brcmf_dbg(TRACE, "enter\n");

	/* expect using firmware supplicant for 1X */
	ifp = netdev_priv(dev);
	if (WARN_ON(ifp->vif->profile.use_fwsup != BRCMF_PROFILE_FWSUP_1X))
		return -EINVAL;

	return brcmf_set_pmk(ifp, conf->pmk, conf->pmk_len);
}
