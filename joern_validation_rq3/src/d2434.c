void ipv4_update_pmtu(struct sk_buff *skb, struct net *net, u32 mtu,
		      int oif, u32 mark, u8 protocol, int flow_flags)
{
	const struct iphdr *iph = (const struct iphdr *) skb->data;
	struct flowi4 fl4;
	struct rtable *rt;

	if (!mark)
		mark = IP4_REPLY_MARK(net, skb->mark);

	__build_flow_key(net, &fl4, NULL, iph, oif,
			 RT_TOS(iph->tos), protocol, mark, flow_flags);
	rt = __ip_route_output_key(net, &fl4);
	if (!IS_ERR(rt)) {
		__ip_rt_update_pmtu(rt, &fl4, mtu);
		ip_rt_put(rt);
	}
}
