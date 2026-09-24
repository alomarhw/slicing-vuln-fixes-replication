static int pppol2tp_getname(struct socket *sock, struct sockaddr *uaddr,
			    int *usockaddr_len, int peer)
{
	int len = 0;
	int error = 0;
	struct l2tp_session *session;
	struct l2tp_tunnel *tunnel;
	struct sock *sk = sock->sk;
	struct inet_sock *inet;
	struct pppol2tp_session *pls;

	error = -ENOTCONN;
	if (sk == NULL)
		goto end;
	if (sk->sk_state != PPPOX_CONNECTED)
		goto end;

	error = -EBADF;
	session = pppol2tp_sock_to_session(sk);
	if (session == NULL)
		goto end;

	pls = l2tp_session_priv(session);
	tunnel = l2tp_sock_to_tunnel(pls->tunnel_sock);
	if (tunnel == NULL) {
		error = -EBADF;
		goto end_put_sess;
	}

	inet = inet_sk(tunnel->sock);
	if ((tunnel->version == 2) && (tunnel->sock->sk_family == AF_INET)) {
		struct sockaddr_pppol2tp sp;
		len = sizeof(sp);
		memset(&sp, 0, len);
		sp.sa_family	= AF_PPPOX;
		sp.sa_protocol	= PX_PROTO_OL2TP;
		sp.pppol2tp.fd  = tunnel->fd;
		sp.pppol2tp.pid = pls->owner;
		sp.pppol2tp.s_tunnel = tunnel->tunnel_id;
		sp.pppol2tp.d_tunnel = tunnel->peer_tunnel_id;
		sp.pppol2tp.s_session = session->session_id;
		sp.pppol2tp.d_session = session->peer_session_id;
		sp.pppol2tp.addr.sin_family = AF_INET;
		sp.pppol2tp.addr.sin_port = inet->inet_dport;
		sp.pppol2tp.addr.sin_addr.s_addr = inet->inet_daddr;
		memcpy(uaddr, &sp, len);
#if IS_ENABLED(CONFIG_IPV6)
	} else if ((tunnel->version == 2) &&
		   (tunnel->sock->sk_family == AF_INET6)) {
		struct sockaddr_pppol2tpin6 sp;

		len = sizeof(sp);
		memset(&sp, 0, len);
		sp.sa_family	= AF_PPPOX;
		sp.sa_protocol	= PX_PROTO_OL2TP;
		sp.pppol2tp.fd  = tunnel->fd;
		sp.pppol2tp.pid = pls->owner;
		sp.pppol2tp.s_tunnel = tunnel->tunnel_id;
		sp.pppol2tp.d_tunnel = tunnel->peer_tunnel_id;
		sp.pppol2tp.s_session = session->session_id;
		sp.pppol2tp.d_session = session->peer_session_id;
		sp.pppol2tp.addr.sin6_family = AF_INET6;
		sp.pppol2tp.addr.sin6_port = inet->inet_dport;
		memcpy(&sp.pppol2tp.addr.sin6_addr, &tunnel->sock->sk_v6_daddr,
		       sizeof(tunnel->sock->sk_v6_daddr));
		memcpy(uaddr, &sp, len);
	} else if ((tunnel->version == 3) &&
		   (tunnel->sock->sk_family == AF_INET6)) {
		struct sockaddr_pppol2tpv3in6 sp;

		len = sizeof(sp);
		memset(&sp, 0, len);
		sp.sa_family	= AF_PPPOX;
		sp.sa_protocol	= PX_PROTO_OL2TP;
		sp.pppol2tp.fd  = tunnel->fd;
		sp.pppol2tp.pid = pls->owner;
		sp.pppol2tp.s_tunnel = tunnel->tunnel_id;
		sp.pppol2tp.d_tunnel = tunnel->peer_tunnel_id;
		sp.pppol2tp.s_session = session->session_id;
		sp.pppol2tp.d_session = session->peer_session_id;
		sp.pppol2tp.addr.sin6_family = AF_INET6;
		sp.pppol2tp.addr.sin6_port = inet->inet_dport;
		memcpy(&sp.pppol2tp.addr.sin6_addr, &tunnel->sock->sk_v6_daddr,
		       sizeof(tunnel->sock->sk_v6_daddr));
		memcpy(uaddr, &sp, len);
#endif
	} else if (tunnel->version == 3) {
		struct sockaddr_pppol2tpv3 sp;
		len = sizeof(sp);
		memset(&sp, 0, len);
		sp.sa_family	= AF_PPPOX;
		sp.sa_protocol	= PX_PROTO_OL2TP;
		sp.pppol2tp.fd  = tunnel->fd;
		sp.pppol2tp.pid = pls->owner;
		sp.pppol2tp.s_tunnel = tunnel->tunnel_id;
		sp.pppol2tp.d_tunnel = tunnel->peer_tunnel_id;
		sp.pppol2tp.s_session = session->session_id;
		sp.pppol2tp.d_session = session->peer_session_id;
		sp.pppol2tp.addr.sin_family = AF_INET;
		sp.pppol2tp.addr.sin_port = inet->inet_dport;
		sp.pppol2tp.addr.sin_addr.s_addr = inet->inet_daddr;
		memcpy(uaddr, &sp, len);
	}

	*usockaddr_len = len;

	sock_put(pls->tunnel_sock);
end_put_sess:
	sock_put(sk);
	error = 0;

end:
	return error;
}
