static void hash_sock_destruct(struct sock *sk)
{
	struct alg_sock *ask = alg_sk(sk);
	struct hash_ctx *ctx = ask->private;

	sock_kzfree_s(sk, ctx->result,
		      crypto_ahash_digestsize(crypto_ahash_reqtfm(&ctx->req)));
	sock_kfree_s(sk, ctx, ctx->len);
	af_alg_release_parent(sk);
}
