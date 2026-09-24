static inline u8 ip4_frag_ecn(u8 tos)
{
	tos = (tos & INET_ECN_MASK) + 1;
	/*
	 * After the last operation we have (in binary):
	 * INET_ECN_NOT_ECT => 001
	 * INET_ECN_ECT_1   => 010
	 * INET_ECN_ECT_0   => 011
	 * INET_ECN_CE      => 100
	 */
	return (tos & 2) ? 0 : tos;
}
