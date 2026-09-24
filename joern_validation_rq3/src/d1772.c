static int llcp_sock_accept(struct socket *sock, struct socket *newsock,
			    int flags)
{
	DECLARE_WAITQUEUE(wait, current);
	struct sock *sk = sock->sk, *new_sk;
	long timeo;
	int ret = 0;

	pr_debug("parent %p\n", sk);

	lock_sock_nested(sk, SINGLE_DEPTH_NESTING);

	if (sk->sk_state != LLCP_LISTEN) {
		ret = -EBADFD;
		goto error;
	}

	timeo = sock_rcvtimeo(sk, flags & O_NONBLOCK);

	/* Wait for an incoming connection. */
	add_wait_queue_exclusive(sk_sleep(sk), &wait);
	while (!(new_sk = nfc_llcp_accept_dequeue(sk, newsock))) {
		set_current_state(TASK_INTERRUPTIBLE);

		if (!timeo) {
			ret = -EAGAIN;
			break;
		}

		if (signal_pending(current)) {
			ret = sock_intr_errno(timeo);
			break;
		}

		release_sock(sk);
		timeo = schedule_timeout(timeo);
		lock_sock_nested(sk, SINGLE_DEPTH_NESTING);
	}
	__set_current_state(TASK_RUNNING);
	remove_wait_queue(sk_sleep(sk), &wait);

	if (ret)
		goto error;

	newsock->state = SS_CONNECTED;

	pr_debug("new socket %p\n", new_sk);

error:
	release_sock(sk);

	return ret;
}
