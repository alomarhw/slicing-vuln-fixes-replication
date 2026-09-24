void __sock_tx_timestamp(const struct sock *sk, __u8 *tx_flags)
{
	u8 flags = *tx_flags;

	if (sk->sk_tsflags & SOF_TIMESTAMPING_TX_HARDWARE)
		flags |= SKBTX_HW_TSTAMP;

	if (sk->sk_tsflags & SOF_TIMESTAMPING_TX_SOFTWARE)
		flags |= SKBTX_SW_TSTAMP;

	if (sk->sk_tsflags & SOF_TIMESTAMPING_TX_SCHED)
		flags |= SKBTX_SCHED_TSTAMP;

	if (sk->sk_tsflags & SOF_TIMESTAMPING_TX_ACK)
		flags |= SKBTX_ACK_TSTAMP;

	*tx_flags = flags;
}
