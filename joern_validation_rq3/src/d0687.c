static int x25_listen(struct socket *sock, int backlog)
{
	struct sock *sk = sock->sk;
	int rc = -EOPNOTSUPP;

	lock_sock(sk);
	if (sk->sk_state != TCP_LISTEN) {
		memset(&x25_sk(sk)->dest_addr, 0, X25_ADDR_LEN);
		sk->sk_max_ack_backlog = backlog;
		sk->sk_state           = TCP_LISTEN;
		rc = 0;
	}
	release_sock(sk);

	return rc;
}
