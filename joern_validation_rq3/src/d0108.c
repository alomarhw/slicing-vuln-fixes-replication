static inline struct sock *udp_v4_mcast_next(struct net *net, struct sock *sk,
					     __be16 loc_port, __be32 loc_addr,
					     __be16 rmt_port, __be32 rmt_addr,
					     int dif)
{
	struct hlist_nulls_node *node;
	struct sock *s = sk;
	unsigned short hnum = ntohs(loc_port);

	sk_nulls_for_each_from(s, node) {
		struct inet_sock *inet = inet_sk(s);

		if (!net_eq(sock_net(s), net) ||
		    udp_sk(s)->udp_port_hash != hnum ||
		    (inet->inet_daddr && inet->inet_daddr != rmt_addr) ||
		    (inet->inet_dport != rmt_port && inet->inet_dport) ||
		    (inet->inet_rcv_saddr &&
		     inet->inet_rcv_saddr != loc_addr) ||
		    ipv6_only_sock(s) ||
		    (s->sk_bound_dev_if && s->sk_bound_dev_if != dif))
			continue;
		if (!ip_mc_sf_allow(s, loc_addr, rmt_addr, dif))
			continue;
		goto found;
	}
	s = NULL;
found:
	return s;
}
