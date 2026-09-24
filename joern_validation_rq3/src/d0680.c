static void mcryptd_hash_final(struct crypto_async_request *req_async, int err)
{
	struct ahash_request *req = ahash_request_cast(req_async);
	struct mcryptd_hash_request_ctx *rctx = ahash_request_ctx(req);

	if (unlikely(err == -EINPROGRESS))
		goto out;

	err = shash_ahash_mcryptd_final(req, &rctx->desc);
	if (err) {
		req->base.complete = rctx->complete;
		goto out;
	}

	return;
out:
	local_bh_disable();
	rctx->complete(&req->base, err);
	local_bh_enable();
}
