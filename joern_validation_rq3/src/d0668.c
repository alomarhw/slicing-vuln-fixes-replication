int ip6_datagram_connect_v6_only(struct sock *sk, struct sockaddr *uaddr,
				 int addr_len)
{
	DECLARE_SOCKADDR(struct sockaddr_in6 *, sin6, uaddr);
	if (sin6->sin6_family != AF_INET6)
		return -EAFNOSUPPORT;
	return ip6_datagram_connect(sk, uaddr, addr_len);
}
