struct ahash_request *mcryptd_ahash_desc(struct ahash_request *req)
{
	struct mcryptd_hash_request_ctx *rctx = ahash_request_ctx(req);
	return &rctx->areq;
}
