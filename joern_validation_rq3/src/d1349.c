static inline struct sock *__udp4_lib_lookup_skb(struct sk_buff *skb,
						 __be16 sport, __be16 dport,
						 struct udp_table *udptable)
{
	const struct iphdr *iph = ip_hdr(skb);

	return __udp4_lib_lookup(dev_net(skb_dst(skb)->dev), iph->saddr, sport,
				 iph->daddr, dport, inet_iif(skb),
				 udptable);
}
