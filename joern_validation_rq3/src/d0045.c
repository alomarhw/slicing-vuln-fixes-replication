static int nfs4_proc_lookup(struct rpc_clnt *clnt, struct inode *dir, struct qstr *name,
			    struct nfs_fh *fhandle, struct nfs_fattr *fattr)
{
	struct nfs4_exception exception = { };
	int err;
	do {
		int status;

		status = _nfs4_proc_lookup(clnt, dir, name, fhandle, fattr);
		switch (status) {
		case -NFS4ERR_BADNAME:
			return -ENOENT;
		case -NFS4ERR_MOVED:
			return nfs4_get_referral(dir, name, fattr, fhandle);
		case -NFS4ERR_WRONGSEC:
			nfs_fixup_secinfo_attributes(fattr, fhandle);
		}
		err = nfs4_handle_exception(NFS_SERVER(dir),
				status, &exception);
	} while (exception.retry);
	return err;
}
