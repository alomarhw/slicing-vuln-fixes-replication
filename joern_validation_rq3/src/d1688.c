static struct wait_queue_head *sock_get_poll_head(struct file *file,
		__poll_t events)
{
	struct socket *sock = file->private_data;

	if (!sock->ops->poll_mask)
		return NULL;
	sock_poll_busy_loop(sock, events);
	return sk_sleep(sock->sk);
}
