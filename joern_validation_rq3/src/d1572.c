hncp_print(netdissect_options *ndo,
           const u_char *cp, u_int length)
{
    ND_PRINT((ndo, "hncp (%d)", length));
    hncp_print_rec(ndo, cp, length, 1);
}
