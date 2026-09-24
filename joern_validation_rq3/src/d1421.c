static NTSTATUS check_base_file_access(struct connection_struct *conn,
				struct smb_filename *smb_fname,
				uint32_t access_mask)
{
	NTSTATUS status;

	status = smbd_calculate_access_mask(conn, smb_fname,
					false,
					access_mask,
					&access_mask);
	if (!NT_STATUS_IS_OK(status)) {
		DEBUG(10, ("smbd_calculate_access_mask "
			"on file %s returned %s\n",
			smb_fname_str_dbg(smb_fname),
			nt_errstr(status)));
		return status;
	}

	if (access_mask & (FILE_WRITE_DATA|FILE_APPEND_DATA)) {
		uint32_t dosattrs;
		if (!CAN_WRITE(conn)) {
			return NT_STATUS_ACCESS_DENIED;
		}
		dosattrs = dos_mode(conn, smb_fname);
		if (IS_DOS_READONLY(dosattrs)) {
			return NT_STATUS_ACCESS_DENIED;
		}
	}

	return smbd_check_access_rights(conn,
					smb_fname,
					false,
					access_mask);
}
