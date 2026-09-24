SMB2_QFS_attr(const unsigned int xid, struct cifs_tcon *tcon,
	      u64 persistent_fid, u64 volatile_fid, int level)
{
	struct smb_rqst rqst;
	struct smb2_query_info_rsp *rsp = NULL;
	struct kvec iov;
	struct kvec rsp_iov;
	int rc = 0;
	int resp_buftype, max_len, min_len;
	struct cifs_ses *ses = tcon->ses;
	unsigned int rsp_len, offset;
	int flags = 0;

	if (level == FS_DEVICE_INFORMATION) {
		max_len = sizeof(FILE_SYSTEM_DEVICE_INFO);
		min_len = sizeof(FILE_SYSTEM_DEVICE_INFO);
	} else if (level == FS_ATTRIBUTE_INFORMATION) {
		max_len = sizeof(FILE_SYSTEM_ATTRIBUTE_INFO);
		min_len = MIN_FS_ATTR_INFO_SIZE;
	} else if (level == FS_SECTOR_SIZE_INFORMATION) {
		max_len = sizeof(struct smb3_fs_ss_info);
		min_len = sizeof(struct smb3_fs_ss_info);
	} else if (level == FS_VOLUME_INFORMATION) {
		max_len = sizeof(struct smb3_fs_vol_info) + MAX_VOL_LABEL_LEN;
		min_len = sizeof(struct smb3_fs_vol_info);
	} else {
		cifs_dbg(FYI, "Invalid qfsinfo level %d\n", level);
		return -EINVAL;
	}

	rc = build_qfs_info_req(&iov, tcon, level, max_len,
				persistent_fid, volatile_fid);
	if (rc)
		return rc;

	if (smb3_encryption_required(tcon))
		flags |= CIFS_TRANSFORM_REQ;

	memset(&rqst, 0, sizeof(struct smb_rqst));
	rqst.rq_iov = &iov;
	rqst.rq_nvec = 1;

	rc = cifs_send_recv(xid, ses, &rqst, &resp_buftype, flags, &rsp_iov);
	cifs_small_buf_release(iov.iov_base);
	if (rc) {
		cifs_stats_fail_inc(tcon, SMB2_QUERY_INFO_HE);
		goto qfsattr_exit;
	}
	rsp = (struct smb2_query_info_rsp *)rsp_iov.iov_base;

	rsp_len = le32_to_cpu(rsp->OutputBufferLength);
	offset = le16_to_cpu(rsp->OutputBufferOffset);
	rc = smb2_validate_iov(offset, rsp_len, &rsp_iov, min_len);
	if (rc)
		goto qfsattr_exit;

	if (level == FS_ATTRIBUTE_INFORMATION)
		memcpy(&tcon->fsAttrInfo, offset
			+ (char *)rsp, min_t(unsigned int,
			rsp_len, max_len));
	else if (level == FS_DEVICE_INFORMATION)
		memcpy(&tcon->fsDevInfo, offset
			+ (char *)rsp, sizeof(FILE_SYSTEM_DEVICE_INFO));
	else if (level == FS_SECTOR_SIZE_INFORMATION) {
		struct smb3_fs_ss_info *ss_info = (struct smb3_fs_ss_info *)
			(offset + (char *)rsp);
		tcon->ss_flags = le32_to_cpu(ss_info->Flags);
		tcon->perf_sector_size =
			le32_to_cpu(ss_info->PhysicalBytesPerSectorForPerf);
	} else if (level == FS_VOLUME_INFORMATION) {
		struct smb3_fs_vol_info *vol_info = (struct smb3_fs_vol_info *)
			(offset + (char *)rsp);
		tcon->vol_serial_number = vol_info->VolumeSerialNumber;
		tcon->vol_create_time = vol_info->VolumeCreationTime;
	}

qfsattr_exit:
	free_rsp_buf(resp_buftype, rsp_iov.iov_base);
	return rc;
}
