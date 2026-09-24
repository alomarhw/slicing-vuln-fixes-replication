void ipc_init_ids(struct ipc_ids *ids)
{
	ids->in_use = 0;
	ids->seq = 0;
	ids->next_id = -1;
	init_rwsem(&ids->rwsem);
	idr_init(&ids->ipcs_idr);
}
