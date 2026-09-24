process_mux_stdio_fwd(u_int rid, Channel *c, Buffer *m, Buffer *r)
{
	Channel *nc;
	char *reserved, *chost;
	u_int cport, i, j;
	int new_fd[2];
	struct mux_stdio_confirm_ctx *cctx;

	chost = reserved = NULL;
	if ((reserved = buffer_get_string_ret(m, NULL)) == NULL ||
	   (chost = buffer_get_string_ret(m, NULL)) == NULL ||
	    buffer_get_int_ret(&cport, m) != 0) {
		free(reserved);
		free(chost);
		error("%s: malformed message", __func__);
		return -1;
	}
	free(reserved);

	debug2("%s: channel %d: request stdio fwd to %s:%u",
	    __func__, c->self, chost, cport);

	/* Gather fds from client */
	for(i = 0; i < 2; i++) {
		if ((new_fd[i] = mm_receive_fd(c->sock)) == -1) {
			error("%s: failed to receive fd %d from slave",
			    __func__, i);
			for (j = 0; j < i; j++)
				close(new_fd[j]);
			free(chost);

			/* prepare reply */
			buffer_put_int(r, MUX_S_FAILURE);
			buffer_put_int(r, rid);
			buffer_put_cstring(r,
			    "did not receive file descriptors");
			return -1;
		}
	}

	debug3("%s: got fds stdin %d, stdout %d", __func__,
	    new_fd[0], new_fd[1]);

	/* XXX support multiple child sessions in future */
	if (c->remote_id != -1) {
		debug2("%s: session already open", __func__);
		/* prepare reply */
		buffer_put_int(r, MUX_S_FAILURE);
		buffer_put_int(r, rid);
		buffer_put_cstring(r, "Multiple sessions not supported");
 cleanup:
		close(new_fd[0]);
		close(new_fd[1]);
		free(chost);
		return 0;
	}

	if (options.control_master == SSHCTL_MASTER_ASK ||
	    options.control_master == SSHCTL_MASTER_AUTO_ASK) {
		if (!ask_permission("Allow forward to %s:%u? ",
		    chost, cport)) {
			debug2("%s: stdio fwd refused by user", __func__);
			/* prepare reply */
			buffer_put_int(r, MUX_S_PERMISSION_DENIED);
			buffer_put_int(r, rid);
			buffer_put_cstring(r, "Permission denied");
			goto cleanup;
		}
	}

	/* enable nonblocking unless tty */
	if (!isatty(new_fd[0]))
		set_nonblock(new_fd[0]);
	if (!isatty(new_fd[1]))
		set_nonblock(new_fd[1]);

	nc = channel_connect_stdio_fwd(chost, cport, new_fd[0], new_fd[1]);

	nc->ctl_chan = c->self;		/* link session -> control channel */
	c->remote_id = nc->self; 	/* link control -> session channel */

	debug2("%s: channel_new: %d linked to control channel %d",
	    __func__, nc->self, nc->ctl_chan);

	channel_register_cleanup(nc->self, mux_master_session_cleanup_cb, 1);

	cctx = xcalloc(1, sizeof(*cctx));
	cctx->rid = rid;
	channel_register_open_confirm(nc->self, mux_stdio_confirm, cctx);
	c->mux_pause = 1; /* stop handling messages until open_confirm done */

	/* reply is deferred, sent by mux_session_confirm */
	return 0;
}
