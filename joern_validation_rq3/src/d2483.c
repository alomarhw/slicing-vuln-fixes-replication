static void tg3_set_rx_mode(struct net_device *dev)
{
	struct tg3 *tp = netdev_priv(dev);

	if (!netif_running(dev))
		return;

	tg3_full_lock(tp, 0);
	__tg3_set_rx_mode(dev);
	tg3_full_unlock(tp);
}
