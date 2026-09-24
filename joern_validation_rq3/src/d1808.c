int ipv6_chk_home_addr(struct net *net, const struct in6_addr *addr)
{
	int ret = 0;
	struct inet6_ifaddr *ifp = NULL;
	unsigned int hash = inet6_addr_hash(addr);

	rcu_read_lock_bh();
	hlist_for_each_entry_rcu_bh(ifp, &inet6_addr_lst[hash], addr_lst) {
		if (!net_eq(dev_net(ifp->idev->dev), net))
			continue;
		if (ipv6_addr_equal(&ifp->addr, addr) &&
		    (ifp->flags & IFA_F_HOMEADDRESS)) {
			ret = 1;
			break;
		}
	}
	rcu_read_unlock_bh();
	return ret;
}
