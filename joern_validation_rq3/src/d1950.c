static int ip6mr_fib_lookup(struct net *net, struct flowi6 *flp6,
			    struct mr6_table **mrt)
{
	*mrt = net->ipv6.mrt6;
	return 0;
}
