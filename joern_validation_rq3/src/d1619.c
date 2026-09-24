static ssize_t skb_unix_socket_splice(struct sock *sk,
				      struct pipe_inode_info *pipe,
				      struct splice_pipe_desc *spd)
{
	int ret;
	struct unix_sock *u = unix_sk(sk);

	mutex_unlock(&u->readlock);
	ret = splice_to_pipe(pipe, spd);
	mutex_lock(&u->readlock);

	return ret;
}
