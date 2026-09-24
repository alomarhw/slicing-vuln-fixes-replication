static void nfs40_clear_delegation_stateid(struct nfs4_state *state)
{
	if (rcu_access_pointer(NFS_I(state->inode)->delegation) != NULL)
		nfs_finish_clear_delegation_stateid(state);
}
