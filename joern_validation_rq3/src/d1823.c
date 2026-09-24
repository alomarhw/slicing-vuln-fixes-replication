daemon_msg_err(SOCKET sockctrl, uint32 plen)
{
	char errbuf[PCAP_ERRBUF_SIZE];
	char remote_errbuf[PCAP_ERRBUF_SIZE];

	if (plen >= PCAP_ERRBUF_SIZE)
	{
		/*
		 * Message is too long; just read as much of it as we
		 * can into the buffer provided, and discard the rest.
		 */
		if (sock_recv(sockctrl, remote_errbuf, PCAP_ERRBUF_SIZE - 1,
		    SOCK_RECEIVEALL_YES|SOCK_EOF_IS_ERROR, errbuf,
		    PCAP_ERRBUF_SIZE) == -1)
		{
			rpcapd_log(LOGPRIO_ERROR, "Read from client failed: %s", errbuf);
			return -1;
		}
		if (rpcapd_discard(sockctrl, plen - (PCAP_ERRBUF_SIZE - 1)) == -1)
		{
			return -1;
		}

		/*
		 * Null-terminate it.
		 */
		remote_errbuf[PCAP_ERRBUF_SIZE - 1] = '\0';
	}
	else if (plen == 0)
	{
		/* Empty error string. */
		remote_errbuf[0] = '\0';
	}
	else
	{
		if (sock_recv(sockctrl, remote_errbuf, plen,
		    SOCK_RECEIVEALL_YES|SOCK_EOF_IS_ERROR, errbuf,
		    PCAP_ERRBUF_SIZE) == -1)
		{
			rpcapd_log(LOGPRIO_ERROR, "Read from client failed: %s", errbuf);
			return -1;
		}

		/*
		 * Null-terminate it.
		 */
		remote_errbuf[plen] = '\0';
	}
	rpcapd_log(LOGPRIO_ERROR, "Error from client: %s", remote_errbuf);
	return 0;
}
