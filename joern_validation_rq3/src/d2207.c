smp_fetch_capture_res_ver(struct proxy *px, struct session *l4, void *l7, unsigned int opt,
                          const struct arg *args, struct sample *smp, const char *kw)
{
	struct http_txn *txn = l7;

	if (txn->rsp.msg_state < HTTP_MSG_HDR_FIRST)
		return 0;

	if (txn->rsp.flags & HTTP_MSGF_VER_11)
		smp->data.str.str = "HTTP/1.1";
	else
		smp->data.str.str = "HTTP/1.0";

	smp->data.str.len = 8;
	smp->type  = SMP_T_STR;
	smp->flags = SMP_F_CONST;
	return 1;

}
