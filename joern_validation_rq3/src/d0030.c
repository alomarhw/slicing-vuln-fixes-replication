static void multi_enable_ms(struct sb_uart_port *port)
{
	struct mp_port *mtpt = (struct mp_port *)port;

	mtpt->ier |= UART_IER_MSI;
	serial_out(mtpt, UART_IER, mtpt->ier);
}
