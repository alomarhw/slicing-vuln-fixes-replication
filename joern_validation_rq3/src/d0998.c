static void list_remove_connection(struct mt_connection *conn) {
	if (connections_head == NULL) {
		return;
	}

	if (conn->state == STATE_ACTIVE && conn->ptsfd > 0) {
		close(conn->ptsfd);
	}
	if (conn->state == STATE_ACTIVE && conn->slavefd > 0) {
		close(conn->slavefd);
	}

	uwtmp_logout(conn);

	DL_DELETE(connections_head, conn);
	free(conn);
}
