static int multi_startup(struct sb_uart_port *port)
{
	struct mp_port *mtpt = (struct mp_port *)port;
	unsigned long flags;
	int retval;

	mtpt->capabilities = uart_config[mtpt->port.type].flags;
	mtpt->mcr = 0;

	if (mtpt->capabilities & UART_CLEAR_FIFO) {
		serial_outp(mtpt, UART_FCR, UART_FCR_ENABLE_FIFO);
		serial_outp(mtpt, UART_FCR, UART_FCR_ENABLE_FIFO |
				UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);
		serial_outp(mtpt, UART_FCR, 0);
	}

	(void) serial_inp(mtpt, UART_LSR);
	(void) serial_inp(mtpt, UART_RX);
	(void) serial_inp(mtpt, UART_IIR);
	(void) serial_inp(mtpt, UART_MSR);
	serial_outp(mtpt, UART_MSR, 0);


	if (!(mtpt->port.flags & UPF_BUGGY_UART) &&
			(serial_inp(mtpt, UART_LSR) == 0xff)) {
		printk("ttyS%d: LSR safety check engaged!\n", mtpt->port.line);
	}

	if ((!is_real_interrupt(mtpt->port.irq)) || (mtpt->poll_type==TYPE_POLL)) {
		unsigned int timeout = mtpt->port.timeout;

		timeout = timeout > 6 ? (timeout / 2 - 2) : 1;

		mtpt->timer.data = (unsigned long)mtpt;
		mod_timer(&mtpt->timer, jiffies + timeout);
	} 
	else 
	{
		retval = serial_link_irq_chain(mtpt);
		if (retval)
			return retval;
	}

	serial_outp(mtpt, UART_LCR, UART_LCR_WLEN8);

	spin_lock_irqsave(&mtpt->port.lock, flags);
	if ((is_real_interrupt(mtpt->port.irq))||(mtpt->poll_type==TYPE_INTERRUPT))
		mtpt->port.mctrl |= TIOCM_OUT2;

	multi_set_mctrl(&mtpt->port, mtpt->port.mctrl);
	spin_unlock_irqrestore(&mtpt->port.lock, flags);

	
	mtpt->ier = UART_IER_RLSI | UART_IER_RDI;
	serial_outp(mtpt, UART_IER, mtpt->ier);

	(void) serial_inp(mtpt, UART_LSR);
	(void) serial_inp(mtpt, UART_RX);
	(void) serial_inp(mtpt, UART_IIR);
	(void) serial_inp(mtpt, UART_MSR);

	return 0;
}
