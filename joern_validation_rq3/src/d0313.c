struct fuse_req *fuse_get_req(struct fuse_conn *fc)
{
	struct fuse_req *req;
	sigset_t oldset;
	int intr;
	int err;

	atomic_inc(&fc->num_waiting);
	block_sigs(&oldset);
	intr = wait_event_interruptible(fc->blocked_waitq, !fc->blocked);
	restore_sigs(&oldset);
	err = -EINTR;
	if (intr)
		goto out;

	err = -ENOTCONN;
	if (!fc->connected)
		goto out;

	req = fuse_request_alloc();
	err = -ENOMEM;
	if (!req)
		goto out;

	fuse_req_init_context(req);
	req->waiting = 1;
	return req;

 out:
	atomic_dec(&fc->num_waiting);
	return ERR_PTR(err);
}
