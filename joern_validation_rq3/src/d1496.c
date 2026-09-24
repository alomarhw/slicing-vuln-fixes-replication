 atmarp_addr_print(netdissect_options *ndo,
 		  const u_char *ha, u_int ha_len, const u_char *srca,
    u_int srca_len)
{
	if (ha_len == 0)
		ND_PRINT((ndo, "<No address>"));
	else {
		ND_PRINT((ndo, "%s", linkaddr_string(ndo, ha, LINKADDR_ATM, ha_len)));
		if (srca_len != 0)
			ND_PRINT((ndo, ",%s",
				  linkaddr_string(ndo, srca, LINKADDR_ATM, srca_len)));
 	}
 }
