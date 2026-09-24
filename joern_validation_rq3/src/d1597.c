static void handle_new_lsr(struct edgeport_port *edge_port, int lsr_data,
							__u8 lsr, __u8 data)
{
	struct async_icount *icount;
	__u8 new_lsr = (__u8)(lsr & (__u8)(LSR_OVER_ERR | LSR_PAR_ERR |
						LSR_FRM_ERR | LSR_BREAK));
	struct tty_struct *tty;

	dev_dbg(&edge_port->port->dev, "%s - %02x\n", __func__, new_lsr);

	edge_port->shadow_lsr = lsr;

	if (new_lsr & LSR_BREAK)
		/*
		 * Parity and Framing errors only count if they
		 * occur exclusive of a break being received.
		 */
		new_lsr &= (__u8)(LSR_OVER_ERR | LSR_BREAK);

	/* Place LSR data byte into Rx buffer */
	if (lsr_data) {
		tty = tty_port_tty_get(&edge_port->port->port);
		if (tty) {
			edge_tty_recv(&edge_port->port->dev, tty, &data, 1);
			tty_kref_put(tty);
		}
	}

	/* update input line counters */
	icount = &edge_port->icount;
	if (new_lsr & LSR_BREAK)
		icount->brk++;
	if (new_lsr & LSR_OVER_ERR)
		icount->overrun++;
	if (new_lsr & LSR_PAR_ERR)
		icount->parity++;
	if (new_lsr & LSR_FRM_ERR)
		icount->frame++;
}
