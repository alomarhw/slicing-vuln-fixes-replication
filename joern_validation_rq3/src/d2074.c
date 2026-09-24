static int __net_init ping_v4_proc_init_net(struct net *net)
{
	return ping_proc_register(net, &ping_v4_seq_afinfo);
}
