static void __net_exit addrconf_exit_net(struct net *net)
{
#ifdef CONFIG_SYSCTL
	__addrconf_sysctl_unregister(net->ipv6.devconf_dflt);
	__addrconf_sysctl_unregister(net->ipv6.devconf_all);
#endif
	kfree(net->ipv6.devconf_dflt);
	kfree(net->ipv6.devconf_all);
}
