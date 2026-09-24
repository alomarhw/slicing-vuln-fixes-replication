netanalyzer_if_print(netdissect_options *ndo, const struct pcap_pkthdr *h,
                     const u_char *p)
{
	/*
	 * Fail if we don't have enough data for the Hilscher pseudo-header.
	 */
	if (h->len < 4 || h->caplen < 4) {
		ND_PRINT((ndo, "[|netanalyzer]"));
		return (h->caplen);
	}

	/* Skip the pseudo-header. */
	return (4 + ether_print(ndo, p + 4, h->len - 4, h->caplen - 4, NULL, NULL));
}
