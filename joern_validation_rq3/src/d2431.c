SMB2_sess_setup(const unsigned int xid, struct cifs_ses *ses,
		const struct nls_table *nls_cp)
{
	int rc = 0;
	struct TCP_Server_Info *server = ses->server;
	struct SMB2_sess_data *sess_data;

	cifs_dbg(FYI, "Session Setup\n");

	if (!server) {
		WARN(1, "%s: server is NULL!\n", __func__);
		return -EIO;
	}

	sess_data = kzalloc(sizeof(struct SMB2_sess_data), GFP_KERNEL);
	if (!sess_data)
		return -ENOMEM;

	rc = SMB2_select_sec(ses, sess_data);
	if (rc)
		goto out;
	sess_data->xid = xid;
	sess_data->ses = ses;
	sess_data->buf0_type = CIFS_NO_BUFFER;
	sess_data->nls_cp = (struct nls_table *) nls_cp;
	sess_data->previous_session = ses->Suid;

	/*
	 * Initialize the session hash with the server one.
	 */
	memcpy(ses->preauth_sha_hash, ses->server->preauth_sha_hash,
	       SMB2_PREAUTH_HASH_SIZE);

	while (sess_data->func)
		sess_data->func(sess_data);

	if ((ses->session_flags & SMB2_SESSION_FLAG_IS_GUEST) && (ses->sign))
		cifs_dbg(VFS, "signing requested but authenticated as guest\n");
	rc = sess_data->result;
out:
	kfree(sess_data);
	return rc;
}
