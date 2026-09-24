lookup_kiocb(struct kioctx *ctx, struct iocb __user *iocb, u32 key)
{
	struct aio_kiocb *kiocb;

	assert_spin_locked(&ctx->ctx_lock);

	if (key != KIOCB_KEY)
		return NULL;

	/* TODO: use a hash or array, this sucks. */
	list_for_each_entry(kiocb, &ctx->active_reqs, ki_list) {
		if (kiocb->ki_user_iocb == iocb)
			return kiocb;
	}
	return NULL;
}
