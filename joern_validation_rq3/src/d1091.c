static void server_connect_callback_init_ssl(SERVER_REC *server, GIOChannel *handle)
{
	int error;

	g_return_if_fail(IS_SERVER(server));

	error = irssi_ssl_handshake(handle);
	if (error == -1) {
		server->connection_lost = TRUE;
		server_connect_failed(server, NULL);
		return;
	}
	if (error & 1) {
		if (server->connect_tag != -1)
			g_source_remove(server->connect_tag);
		server->connect_tag = g_input_add(handle, error == 1 ? G_INPUT_READ : G_INPUT_WRITE,
						  (GInputFunction)
						  server_connect_callback_init_ssl,
						  server);
		return;
	}

	lookup_servers = g_slist_remove(lookup_servers, server);
	if (server->connect_tag != -1) {
		g_source_remove(server->connect_tag);
		server->connect_tag = -1;
	}

	server_connect_finished(server);
}
