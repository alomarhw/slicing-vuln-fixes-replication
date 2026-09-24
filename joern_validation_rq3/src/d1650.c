static void stream_int_shutr_conn(struct stream_interface *si)
{
	struct connection *conn = __objt_conn(si->end);

	si->ib->flags &= ~CF_SHUTR_NOW;
	if (si->ib->flags & CF_SHUTR)
		return;
	si->ib->flags |= CF_SHUTR;
	si->ib->rex = TICK_ETERNITY;
	si->flags &= ~SI_FL_WAIT_ROOM;

	if (si->state != SI_ST_EST && si->state != SI_ST_CON)
		return;

	if (si->ob->flags & CF_SHUTW) {
		conn_full_close(conn);
		si->state = SI_ST_DIS;
		si->exp = TICK_ETERNITY;
	}
	else if (si->flags & SI_FL_NOHALF) {
		/* we want to immediately forward this close to the write side */
		return stream_int_shutw_conn(si);
	}
	else if (conn->ctrl) {
		/* we want the caller to disable polling on this FD */
		conn_data_stop_recv(conn);
	}
}
