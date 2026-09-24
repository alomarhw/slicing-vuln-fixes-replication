static unsigned int udp6_ehashfn(struct net *net,
				  const struct in6_addr *laddr,
				  const u16 lport,
				  const struct in6_addr *faddr,
				  const __be16 fport)
{
	static u32 udp6_ehash_secret __read_mostly;
	static u32 udp_ipv6_hash_secret __read_mostly;

	u32 lhash, fhash;

	net_get_random_once(&udp6_ehash_secret,
			    sizeof(udp6_ehash_secret));
	net_get_random_once(&udp_ipv6_hash_secret,
			    sizeof(udp_ipv6_hash_secret));

	lhash = (__force u32)laddr->s6_addr32[3];
	fhash = __ipv6_addr_jhash(faddr, udp_ipv6_hash_secret);

	return __inet6_ehashfn(lhash, lport, fhash, fport,
			       udp_ipv6_hash_secret + net_hash_mix(net));
}
