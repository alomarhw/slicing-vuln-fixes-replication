int __init if6_proc_init(void)
{
	return register_pernet_subsys(&if6_proc_net_ops);
}
