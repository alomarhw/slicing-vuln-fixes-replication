static void __net_exit netlink_net_exit(struct net *net)
{
#ifdef CONFIG_PROC_FS
	proc_net_remove(net, "netlink");
#endif
}
