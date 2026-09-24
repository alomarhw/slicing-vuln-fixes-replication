void ipv4_sk_redirect(struct sk_buff *skb, struct sock *sk)
{
	const struct iphdr *iph = (const struct iphdr *) skb->data;
	struct flowi4 fl4;
	struct rtable *rt;
	struct net *net = sock_net(sk);

	__build_flow_key(net, &fl4, sk, iph, 0, 0, 0, 0, 0);
	rt = __ip_route_output_key(net, &fl4);
	if (!IS_ERR(rt)) {
		__ip_do_redirect(rt, skb, &fl4, false);
		ip_rt_put(rt);
	}
}
