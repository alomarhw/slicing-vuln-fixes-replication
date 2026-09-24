void tcp_proc_unregister(struct net *net, struct tcp_seq_afinfo *afinfo)
{
	proc_net_remove(net, afinfo->name);
}
