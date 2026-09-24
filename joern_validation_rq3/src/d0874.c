static int sock_setbindtodevice(struct sock *sk, char __user *optval,
				int optlen)
{
	int ret = -ENOPROTOOPT;
#ifdef CONFIG_NETDEVICES
	struct net *net = sock_net(sk);
	char devname[IFNAMSIZ];
	int index;

	/* Sorry... */
	ret = -EPERM;
	if (!ns_capable(net->user_ns, CAP_NET_RAW))
		goto out;

	ret = -EINVAL;
	if (optlen < 0)
		goto out;

	/* Bind this socket to a particular device like "eth0",
	 * as specified in the passed interface name. If the
	 * name is "" or the option length is zero the socket
	 * is not bound.
	 */
	if (optlen > IFNAMSIZ - 1)
		optlen = IFNAMSIZ - 1;
	memset(devname, 0, sizeof(devname));

	ret = -EFAULT;
	if (copy_from_user(devname, optval, optlen))
		goto out;

	index = 0;
	if (devname[0] != '\0') {
		struct net_device *dev;

		rcu_read_lock();
		dev = dev_get_by_name_rcu(net, devname);
		if (dev)
			index = dev->ifindex;
		rcu_read_unlock();
		ret = -ENODEV;
		if (!dev)
			goto out;
	}

	lock_sock(sk);
	sk->sk_bound_dev_if = index;
	sk_dst_reset(sk);
	release_sock(sk);

	ret = 0;

out:
#endif

	return ret;
}
