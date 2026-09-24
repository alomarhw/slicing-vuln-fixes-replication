parse_lease_state(struct TCP_Server_Info *server, struct smb2_create_rsp *rsp,
		  unsigned int *epoch)
{
	char *data_offset;
	struct create_context *cc;
	unsigned int next = 0;
	char *name;

	data_offset = (char *)rsp + 4 + le32_to_cpu(rsp->CreateContextsOffset);
	cc = (struct create_context *)data_offset;
	do {
		cc = (struct create_context *)((char *)cc + next);
		name = le16_to_cpu(cc->NameOffset) + (char *)cc;
		if (le16_to_cpu(cc->NameLength) != 4 ||
		    strncmp(name, "RqLs", 4)) {
			next = le32_to_cpu(cc->Next);
			continue;
		}
		return server->ops->parse_lease_buf(cc, epoch);
	} while (next != 0);

	return 0;
}
