static int __net_init addrconf_init_net(struct net *net)
{
	int err = -ENOMEM;
	struct ipv6_devconf *all, *dflt;

	all = kmemdup(&ipv6_devconf, sizeof(ipv6_devconf), GFP_KERNEL);
	if (all == NULL)
		goto err_alloc_all;

	dflt = kmemdup(&ipv6_devconf_dflt, sizeof(ipv6_devconf_dflt), GFP_KERNEL);
	if (dflt == NULL)
		goto err_alloc_dflt;

	/* these will be inherited by all namespaces */
	dflt->autoconf = ipv6_defaults.autoconf;
	dflt->disable_ipv6 = ipv6_defaults.disable_ipv6;

	net->ipv6.devconf_all = all;
	net->ipv6.devconf_dflt = dflt;

#ifdef CONFIG_SYSCTL
	err = __addrconf_sysctl_register(net, "all", NULL, all);
	if (err < 0)
		goto err_reg_all;

	err = __addrconf_sysctl_register(net, "default", NULL, dflt);
	if (err < 0)
		goto err_reg_dflt;
#endif
	return 0;

#ifdef CONFIG_SYSCTL
err_reg_dflt:
	__addrconf_sysctl_unregister(all);
err_reg_all:
	kfree(dflt);
#endif
err_alloc_dflt:
	kfree(all);
err_alloc_all:
	return err;
}
