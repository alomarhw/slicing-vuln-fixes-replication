static bool netlink_skb_is_mmaped(const struct sk_buff *skb)
{
	return NETLINK_CB(skb).flags & NETLINK_SKB_MMAPED;
}
