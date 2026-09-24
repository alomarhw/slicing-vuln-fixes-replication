data_sock_bind(struct socket *sock, struct sockaddr *addr, int addr_len)
{
	struct sockaddr_mISDN *maddr = (struct sockaddr_mISDN *) addr;
	struct sock *sk = sock->sk;
	struct sock *csk;
	int err = 0;

	if (*debug & DEBUG_SOCKET)
		printk(KERN_DEBUG "%s(%p) sk=%p\n", __func__, sock, sk);
	if (addr_len != sizeof(struct sockaddr_mISDN))
		return -EINVAL;
	if (!maddr || maddr->family != AF_ISDN)
		return -EINVAL;

	lock_sock(sk);

	if (_pms(sk)->dev) {
		err = -EALREADY;
		goto done;
	}
	_pms(sk)->dev = get_mdevice(maddr->dev);
	if (!_pms(sk)->dev) {
		err = -ENODEV;
		goto done;
	}

	if (sk->sk_protocol < ISDN_P_B_START) {
		read_lock_bh(&data_sockets.lock);
		sk_for_each(csk, &data_sockets.head) {
			if (sk == csk)
				continue;
			if (_pms(csk)->dev != _pms(sk)->dev)
				continue;
			if (csk->sk_protocol >= ISDN_P_B_START)
				continue;
			if (IS_ISDN_P_TE(csk->sk_protocol)
			    == IS_ISDN_P_TE(sk->sk_protocol))
				continue;
			read_unlock_bh(&data_sockets.lock);
			err = -EBUSY;
			goto done;
		}
		read_unlock_bh(&data_sockets.lock);
	}

	_pms(sk)->ch.send = mISDN_send;
	_pms(sk)->ch.ctrl = mISDN_ctrl;

	switch (sk->sk_protocol) {
	case ISDN_P_TE_S0:
	case ISDN_P_NT_S0:
	case ISDN_P_TE_E1:
	case ISDN_P_NT_E1:
		mISDN_sock_unlink(&data_sockets, sk);
		err = connect_layer1(_pms(sk)->dev, &_pms(sk)->ch,
				     sk->sk_protocol, maddr);
		if (err)
			mISDN_sock_link(&data_sockets, sk);
		break;
	case ISDN_P_LAPD_TE:
	case ISDN_P_LAPD_NT:
		err = create_l2entity(_pms(sk)->dev, &_pms(sk)->ch,
				      sk->sk_protocol, maddr);
		break;
	case ISDN_P_B_RAW:
	case ISDN_P_B_HDLC:
	case ISDN_P_B_X75SLP:
	case ISDN_P_B_L2DTMF:
	case ISDN_P_B_L2DSP:
	case ISDN_P_B_L2DSPHDLC:
		err = connect_Bstack(_pms(sk)->dev, &_pms(sk)->ch,
				     sk->sk_protocol, maddr);
		break;
	default:
		err = -EPROTONOSUPPORT;
	}
	if (err)
		goto done;
	sk->sk_state = MISDN_BOUND;
	_pms(sk)->ch.protocol = sk->sk_protocol;

done:
	release_sock(sk);
	return err;
}
