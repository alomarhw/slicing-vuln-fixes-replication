static inline unsigned int ip_vs_svc_fwm_hashkey(struct net *net, __u32 fwmark)
{
	return (((size_t)net>>8) ^ fwmark) & IP_VS_SVC_TAB_MASK;
}
