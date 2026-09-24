show_status (int dtr_up) 
{
	int baud, bits;
	enum flowcntrl_e flow;
	enum parity_e parity;

	term_refresh(tty_fd);

	baud = term_get_baudrate(tty_fd, NULL);
	flow = term_get_flowcntrl(tty_fd);
	parity = term_get_parity(tty_fd);
	bits = term_get_databits(tty_fd);
	
	fd_printf(STO, "\r\n");
 
	if ( baud != opts.baud ) {
		fd_printf(STO, "*** baud: %d (%d)\r\n", opts.baud, baud);
	} else { 
		fd_printf(STO, "*** baud: %d\r\n", opts.baud);
	}
	if ( flow != opts.flow ) {
		fd_printf(STO, "*** flow: %s (%s)\r\n", 
				  flow_str[opts.flow], flow_str[flow]);
	} else {
		fd_printf(STO, "*** flow: %s\r\n", flow_str[opts.flow]);
	}
	if ( parity != opts.parity ) {
		fd_printf(STO, "*** parity: %s (%s)\r\n", 
				  parity_str[opts.parity], parity_str[parity]);
	} else {
		fd_printf(STO, "*** parity: %s\r\n", parity_str[opts.parity]);
	}
	if ( bits != opts.databits ) {
		fd_printf(STO, "*** databits: %d (%d)\r\n", opts.databits, bits);
	} else {
		fd_printf(STO, "*** databits: %d\r\n", opts.databits);
	}
	fd_printf(STO, "*** dtr: %s\r\n", dtr_up ? "up" : "down");
}
