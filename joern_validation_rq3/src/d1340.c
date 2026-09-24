static bool smb2cli_req_cancel(struct tevent_req *req)
{
	struct smbXcli_req_state *state =
		tevent_req_data(req,
		struct smbXcli_req_state);
	struct smbXcli_tcon *tcon = state->tcon;
	struct smbXcli_session *session = state->session;
	uint8_t *fixed = state->smb2.pad;
	uint16_t fixed_len = 4;
	struct tevent_req *subreq;
	struct smbXcli_req_state *substate;
	NTSTATUS status;

	SSVAL(fixed, 0, 0x04);
	SSVAL(fixed, 2, 0);

	subreq = smb2cli_req_create(state, state->ev,
				    state->conn,
				    SMB2_OP_CANCEL,
				    0, 0, /* flags */
				    0, /* timeout */
				    tcon, session,
				    fixed, fixed_len,
				    NULL, 0, 0);
	if (subreq == NULL) {
		return false;
	}
	substate = tevent_req_data(subreq, struct smbXcli_req_state);

	SIVAL(substate->smb2.hdr, SMB2_HDR_FLAGS, state->smb2.cancel_flags);
	SBVAL(substate->smb2.hdr, SMB2_HDR_MESSAGE_ID, state->smb2.cancel_mid);
	SBVAL(substate->smb2.hdr, SMB2_HDR_ASYNC_ID, state->smb2.cancel_aid);

	status = smb2cli_req_compound_submit(&subreq, 1);
	if (!NT_STATUS_IS_OK(status)) {
		TALLOC_FREE(subreq);
		return false;
	}

	tevent_req_set_callback(subreq, smb2cli_req_cancel_done, NULL);

	return true;
}
