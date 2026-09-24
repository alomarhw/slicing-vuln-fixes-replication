mux_client_forward(int fd, int cancel_flag, u_int ftype, struct Forward *fwd)
{
	Buffer m;
	char *e, *fwd_desc;
	u_int type, rid;

	fwd_desc = format_forward(ftype, fwd);
	debug("Requesting %s %s",
	    cancel_flag ? "cancellation of" : "forwarding of", fwd_desc);
	free(fwd_desc);

	buffer_init(&m);
	buffer_put_int(&m, cancel_flag ? MUX_C_CLOSE_FWD : MUX_C_OPEN_FWD);
	buffer_put_int(&m, muxclient_request_id);
	buffer_put_int(&m, ftype);
	if (fwd->listen_path != NULL) {
		buffer_put_cstring(&m, fwd->listen_path);
	} else {
		buffer_put_cstring(&m,
		    fwd->listen_host == NULL ? "" :
		    (*fwd->listen_host == '\0' ? "*" : fwd->listen_host));
	}
	buffer_put_int(&m, fwd->listen_port);
	if (fwd->connect_path != NULL) {
		buffer_put_cstring(&m, fwd->connect_path);
	} else {
		buffer_put_cstring(&m,
		    fwd->connect_host == NULL ? "" : fwd->connect_host);
	}
	buffer_put_int(&m, fwd->connect_port);

	if (mux_client_write_packet(fd, &m) != 0)
		fatal("%s: write packet: %s", __func__, strerror(errno));

	buffer_clear(&m);

	/* Read their reply */
	if (mux_client_read_packet(fd, &m) != 0) {
		buffer_free(&m);
		return -1;
	}

	type = buffer_get_int(&m);
	if ((rid = buffer_get_int(&m)) != muxclient_request_id)
		fatal("%s: out of sequence reply: my id %u theirs %u",
		    __func__, muxclient_request_id, rid);
	switch (type) {
	case MUX_S_OK:
		break;
	case MUX_S_REMOTE_PORT:
		if (cancel_flag)
			fatal("%s: got MUX_S_REMOTE_PORT for cancel", __func__);
		fwd->allocated_port = buffer_get_int(&m);
		verbose("Allocated port %u for remote forward to %s:%d",
		    fwd->allocated_port,
		    fwd->connect_host ? fwd->connect_host : "",
		    fwd->connect_port);
		if (muxclient_command == SSHMUX_COMMAND_FORWARD)
			fprintf(stdout, "%i\n", fwd->allocated_port);
		break;
	case MUX_S_PERMISSION_DENIED:
		e = buffer_get_string(&m, NULL);
		buffer_free(&m);
		error("Master refused forwarding request: %s", e);
		return -1;
	case MUX_S_FAILURE:
		e = buffer_get_string(&m, NULL);
		buffer_free(&m);
		error("%s: forwarding request failed: %s", __func__, e);
		return -1;
	default:
		fatal("%s: unexpected response from master 0x%08x",
		    __func__, type);
	}
	buffer_free(&m);

	muxclient_request_id++;
	return 0;
}
