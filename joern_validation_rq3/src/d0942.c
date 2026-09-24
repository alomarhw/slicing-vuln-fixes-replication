static bool __udp_v6_is_mcast_sock(struct net *net, struct sock *sk,
				   __be16 loc_port, const struct in6_addr *loc_addr,
				   __be16 rmt_port, const struct in6_addr *rmt_addr,
				   int dif, unsigned short hnum)
{
	struct inet_sock *inet = inet_sk(sk);

	if (!net_eq(sock_net(sk), net))
		return false;

	if (udp_sk(sk)->udp_port_hash != hnum ||
	    sk->sk_family != PF_INET6 ||
	    (inet->inet_dport && inet->inet_dport != rmt_port) ||
	    (!ipv6_addr_any(&sk->sk_v6_daddr) &&
		    !ipv6_addr_equal(&sk->sk_v6_daddr, rmt_addr)) ||
	    (sk->sk_bound_dev_if && sk->sk_bound_dev_if != dif) ||
	    (!ipv6_addr_any(&sk->sk_v6_rcv_saddr) &&
		    !ipv6_addr_equal(&sk->sk_v6_rcv_saddr, loc_addr)))
		return false;
	if (!inet6_mc_check(sk, loc_addr, rmt_addr))
		return false;
	return true;
}
