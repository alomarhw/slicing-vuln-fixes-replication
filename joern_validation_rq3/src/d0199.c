rsvp_intserv_print(netdissect_options *ndo,
                   const u_char *tptr, u_short obj_tlen)
{
    int parameter_id,parameter_length;
    union {
	float f;
	uint32_t i;
    } bw;

    if (obj_tlen < 4)
        return 0;
    parameter_id = *(tptr);
    ND_TCHECK2(*(tptr + 2), 2);
    parameter_length = EXTRACT_16BITS(tptr+2)<<2; /* convert wordcount to bytecount */

    ND_PRINT((ndo, "\n\t      Parameter ID: %s (%u), length: %u, Flags: [0x%02x]",
           tok2str(rsvp_intserv_parameter_id_values,"unknown",parameter_id),
           parameter_id,
           parameter_length,
           *(tptr + 1)));

    if (obj_tlen < parameter_length+4)
        return 0;
    switch(parameter_id) { /* parameter_id */

    case 4:
       /*
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |    4 (e)      |    (f)        |           1 (g)               |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |        IS hop cnt (32-bit unsigned integer)                   |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        */
        if (parameter_length == 4) {
	    ND_TCHECK2(*(tptr + 4), 4);
            ND_PRINT((ndo, "\n\t\tIS hop count: %u", EXTRACT_32BITS(tptr + 4)));
        }
        break;

    case 6:
       /*
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |    6 (h)      |    (i)        |           1 (j)               |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |  Path b/w estimate  (32-bit IEEE floating point number)       |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        */
        if (parameter_length == 4) {
	    ND_TCHECK2(*(tptr + 4), 4);
            bw.i = EXTRACT_32BITS(tptr+4);
            ND_PRINT((ndo, "\n\t\tPath b/w estimate: %.10g Mbps", bw.f / 125000));
        }
        break;

    case 8:
       /*
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |     8 (k)     |    (l)        |           1 (m)               |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |        Minimum path latency (32-bit integer)                  |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        */
        if (parameter_length == 4) {
	    ND_TCHECK2(*(tptr + 4), 4);
            ND_PRINT((ndo, "\n\t\tMinimum path latency: "));
            if (EXTRACT_32BITS(tptr+4) == 0xffffffff)
                ND_PRINT((ndo, "don't care"));
            else
                ND_PRINT((ndo, "%u", EXTRACT_32BITS(tptr + 4)));
        }
        break;

    case 10:

       /*
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |     10 (n)    |      (o)      |           1 (p)               |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |      Composed MTU (32-bit unsigned integer)                   |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        */
        if (parameter_length == 4) {
	    ND_TCHECK2(*(tptr + 4), 4);
            ND_PRINT((ndo, "\n\t\tComposed MTU: %u bytes", EXTRACT_32BITS(tptr + 4)));
        }
        break;
    case 127:
       /*
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |   127 (e)     |    0 (f)      |             5 (g)             |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |  Token Bucket Rate [r] (32-bit IEEE floating point number)    |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |  Token Bucket Size [b] (32-bit IEEE floating point number)    |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |  Peak Data Rate [p] (32-bit IEEE floating point number)       |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |  Minimum Policed Unit [m] (32-bit integer)                    |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |  Maximum Packet Size [M]  (32-bit integer)                    |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        */

        if (parameter_length == 20) {
	    ND_TCHECK2(*(tptr + 4), 20);
            bw.i = EXTRACT_32BITS(tptr+4);
            ND_PRINT((ndo, "\n\t\tToken Bucket Rate: %.10g Mbps", bw.f / 125000));
            bw.i = EXTRACT_32BITS(tptr+8);
            ND_PRINT((ndo, "\n\t\tToken Bucket Size: %.10g bytes", bw.f));
            bw.i = EXTRACT_32BITS(tptr+12);
            ND_PRINT((ndo, "\n\t\tPeak Data Rate: %.10g Mbps", bw.f / 125000));
            ND_PRINT((ndo, "\n\t\tMinimum Policed Unit: %u bytes", EXTRACT_32BITS(tptr + 16)));
            ND_PRINT((ndo, "\n\t\tMaximum Packet Size: %u bytes", EXTRACT_32BITS(tptr + 20)));
        }
        break;

    case 130:
       /*
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |     130 (h)   |    0 (i)      |            2 (j)              |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |  Rate [R]  (32-bit IEEE floating point number)                |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        * |  Slack Term [S]  (32-bit integer)                             |
        * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        */

        if (parameter_length == 8) {
	    ND_TCHECK2(*(tptr + 4), 8);
            bw.i = EXTRACT_32BITS(tptr+4);
            ND_PRINT((ndo, "\n\t\tRate: %.10g Mbps", bw.f / 125000));
            ND_PRINT((ndo, "\n\t\tSlack Term: %u", EXTRACT_32BITS(tptr + 8)));
        }
        break;

    case 133:
    case 134:
    case 135:
    case 136:
        if (parameter_length == 4) {
	    ND_TCHECK2(*(tptr + 4), 4);
            ND_PRINT((ndo, "\n\t\tValue: %u", EXTRACT_32BITS(tptr + 4)));
        }
        break;

    default:
        if (ndo->ndo_vflag <= 1)
            print_unknown_data(ndo, tptr + 4, "\n\t\t", parameter_length);
    }
    return (parameter_length+4); /* header length 4 bytes */

trunc:
    ND_PRINT((ndo, "%s", tstr));
    return 0;
}
