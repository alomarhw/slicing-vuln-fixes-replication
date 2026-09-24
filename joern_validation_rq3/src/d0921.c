smb2_async_writev(struct cifs_writedata *wdata,
		  void (*release)(struct kref *kref))
{
	int rc = -EACCES, flags = 0;
	struct smb2_write_req *req = NULL;
	struct smb2_sync_hdr *shdr;
	struct cifs_tcon *tcon = tlink_tcon(wdata->cfile->tlink);
	struct TCP_Server_Info *server = tcon->ses->server;
	struct kvec iov[2];
	struct smb_rqst rqst = { };

	rc = small_smb2_init(SMB2_WRITE, tcon, (void **) &req);
	if (rc) {
		if (rc == -EAGAIN && wdata->credits) {
			/* credits was reset by reconnect */
			wdata->credits = 0;
			/* reduce in_flight value since we won't send the req */
			spin_lock(&server->req_lock);
			server->in_flight--;
			spin_unlock(&server->req_lock);
		}
		goto async_writev_out;
	}

	if (encryption_required(tcon))
		flags |= CIFS_TRANSFORM_REQ;

	shdr = get_sync_hdr(req);
	shdr->ProcessId = cpu_to_le32(wdata->cfile->pid);

	req->PersistentFileId = wdata->cfile->fid.persistent_fid;
	req->VolatileFileId = wdata->cfile->fid.volatile_fid;
	req->WriteChannelInfoOffset = 0;
	req->WriteChannelInfoLength = 0;
	req->Channel = 0;
	req->Offset = cpu_to_le64(wdata->offset);
	/* 4 for rfc1002 length field */
	req->DataOffset = cpu_to_le16(
				offsetof(struct smb2_write_req, Buffer) - 4);
	req->RemainingBytes = 0;

	/* 4 for rfc1002 length field and 1 for Buffer */
	iov[0].iov_len = 4;
	iov[0].iov_base = req;
	iov[1].iov_len = get_rfc1002_length(req) - 1;
	iov[1].iov_base = (char *)req + 4;

	rqst.rq_iov = iov;
	rqst.rq_nvec = 2;
	rqst.rq_pages = wdata->pages;
	rqst.rq_npages = wdata->nr_pages;
	rqst.rq_pagesz = wdata->pagesz;
	rqst.rq_tailsz = wdata->tailsz;

	cifs_dbg(FYI, "async write at %llu %u bytes\n",
		 wdata->offset, wdata->bytes);

	req->Length = cpu_to_le32(wdata->bytes);

	inc_rfc1001_len(&req->hdr, wdata->bytes - 1 /* Buffer */);

	if (wdata->credits) {
		shdr->CreditCharge = cpu_to_le16(DIV_ROUND_UP(wdata->bytes,
						    SMB2_MAX_BUFFER_SIZE));
		shdr->CreditRequest = shdr->CreditCharge;
		spin_lock(&server->req_lock);
		server->credits += wdata->credits -
						le16_to_cpu(shdr->CreditCharge);
		spin_unlock(&server->req_lock);
		wake_up(&server->request_q);
		flags |= CIFS_HAS_CREDITS;
	}

	kref_get(&wdata->refcount);
	rc = cifs_call_async(server, &rqst, NULL, smb2_writev_callback, wdata,
			     flags);

	if (rc) {
		kref_put(&wdata->refcount, release);
		cifs_stats_fail_inc(tcon, SMB2_WRITE_HE);
	}

async_writev_out:
	cifs_small_buf_release(req);
	return rc;
}
