static int rpc_check_reply(struct packet *pkt, int rpc_prog,
			   uint32_t rpc_id, int *nfserr)
{
	uint32_t *data;
	struct rpc_reply rpc;

	*nfserr = 0;

	if (!pkt)
		return -EAGAIN;

	memcpy(&rpc, pkt->data, sizeof(rpc));

	if (ntoh32(rpc.id) != rpc_id) {
		if (rpc_id - ntoh32(rpc.id) == 1)
			/* stale packet, wait a bit longer */
			return 0;

		return -EINVAL;
	}

	if (rpc.rstatus  ||
	    rpc.verifier ||
	    rpc.astatus ) {
		return -EINVAL;
	}

	if (rpc_prog != PROG_NFS)
		return 0;

	data = (uint32_t *)(pkt->data + sizeof(struct rpc_reply));
	*nfserr = ntoh32(net_read_uint32(data));
	*nfserr = -*nfserr;

	debug("%s: state: %d, err %d\n", __func__, nfs_state, *nfserr);

	return 0;
}
