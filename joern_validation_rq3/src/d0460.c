signal_handler(int sig)
{
	received_signal = sig;
	quit_pending = 1;
}
