static int hash_accept_parent(void *private, struct sock *sk)
{
	struct hash_ctx *ctx;
	struct alg_sock *ask = alg_sk(sk);
	unsigned len = sizeof(*ctx) + crypto_ahash_reqsize(private);
	unsigned ds = crypto_ahash_digestsize(private);

	ctx = sock_kmalloc(sk, len, GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->result = sock_kmalloc(sk, ds, GFP_KERNEL);
	if (!ctx->result) {
		sock_kfree_s(sk, ctx, len);
		return -ENOMEM;
	}

	memset(ctx->result, 0, ds);

	ctx->len = len;
	ctx->more = 0;
	af_alg_init_completion(&ctx->completion);

	ask->private = ctx;

	ahash_request_set_tfm(&ctx->req, private);
	ahash_request_set_callback(&ctx->req, CRYPTO_TFM_REQ_MAY_BACKLOG,
				   af_alg_complete, &ctx->completion);

	sk->sk_destruct = hash_sock_destruct;

	return 0;
}
