struct net_device *alloc_netdev_mq(int sizeof_priv, const char *name,
		void (*setup)(struct net_device *), unsigned int queue_count)
{
	struct netdev_queue *tx;
	struct net_device *dev;
	size_t alloc_size;
	struct net_device *p;
#ifdef CONFIG_RPS
	struct netdev_rx_queue *rx;
	int i;
#endif

	BUG_ON(strlen(name) >= sizeof(dev->name));

	alloc_size = sizeof(struct net_device);
	if (sizeof_priv) {
		/* ensure 32-byte alignment of private area */
		alloc_size = ALIGN(alloc_size, NETDEV_ALIGN);
		alloc_size += sizeof_priv;
	}
	/* ensure 32-byte alignment of whole construct */
	alloc_size += NETDEV_ALIGN - 1;

	p = kzalloc(alloc_size, GFP_KERNEL);
	if (!p) {
		printk(KERN_ERR "alloc_netdev: Unable to allocate device.\n");
		return NULL;
	}

	tx = kcalloc(queue_count, sizeof(struct netdev_queue), GFP_KERNEL);
	if (!tx) {
		printk(KERN_ERR "alloc_netdev: Unable to allocate "
		       "tx qdiscs.\n");
		goto free_p;
	}

#ifdef CONFIG_RPS
	rx = kcalloc(queue_count, sizeof(struct netdev_rx_queue), GFP_KERNEL);
	if (!rx) {
		printk(KERN_ERR "alloc_netdev: Unable to allocate "
		       "rx queues.\n");
		goto free_tx;
	}

	atomic_set(&rx->count, queue_count);

	/*
	 * Set a pointer to first element in the array which holds the
	 * reference count.
	 */
	for (i = 0; i < queue_count; i++)
		rx[i].first = rx;
#endif

	dev = PTR_ALIGN(p, NETDEV_ALIGN);
	dev->padded = (char *)dev - (char *)p;

	if (dev_addr_init(dev))
		goto free_rx;

	dev_mc_init(dev);
	dev_uc_init(dev);

	dev_net_set(dev, &init_net);

	dev->_tx = tx;
	dev->num_tx_queues = queue_count;
	dev->real_num_tx_queues = queue_count;

#ifdef CONFIG_RPS
	dev->_rx = rx;
	dev->num_rx_queues = queue_count;
#endif

	dev->gso_max_size = GSO_MAX_SIZE;

	netdev_init_queues(dev);

	INIT_LIST_HEAD(&dev->ethtool_ntuple_list.list);
	dev->ethtool_ntuple_list.count = 0;
	INIT_LIST_HEAD(&dev->napi_list);
	INIT_LIST_HEAD(&dev->unreg_list);
	INIT_LIST_HEAD(&dev->link_watch_list);
	dev->priv_flags = IFF_XMIT_DST_RELEASE;
	setup(dev);
	strcpy(dev->name, name);
	return dev;

free_rx:
#ifdef CONFIG_RPS
	kfree(rx);
free_tx:
#endif
	kfree(tx);
free_p:
	kfree(p);
	return NULL;
}
