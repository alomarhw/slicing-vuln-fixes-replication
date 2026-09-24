static void tcp_tx_timestamp(struct sock *sk, u16 tsflags, struct sk_buff *skb)
{
	if (tsflags && skb) {
		struct skb_shared_info *shinfo = skb_shinfo(skb);
		struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);

		sock_tx_timestamp(sk, tsflags, &shinfo->tx_flags);
		if (tsflags & SOF_TIMESTAMPING_TX_ACK)
			tcb->txstamp_ack = 1;
		if (tsflags & SOF_TIMESTAMPING_TX_RECORD_MASK)
			shinfo->tskey = TCP_SKB_CB(skb)->seq + skb->len - 1;
	}
}
