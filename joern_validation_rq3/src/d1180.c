static int nfs4_read_done_cb(struct rpc_task *task, struct nfs_read_data *data)
{
	struct nfs_server *server = NFS_SERVER(data->header->inode);

	if (nfs4_async_handle_error(task, server, data->args.context->state) == -EAGAIN) {
		rpc_restart_call_prepare(task);
		return -EAGAIN;
	}

	__nfs4_read_done_cb(data);
	if (task->tk_status > 0)
		renew_lease(server, data->timestamp);
	return 0;
}
