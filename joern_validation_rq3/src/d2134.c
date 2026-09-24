static Bignum ssh1_pkt_getmp(struct Packet *pkt)
{
    int j;
    Bignum b;

    j = ssh1_read_bignum(pkt->body + pkt->savedpos,
			 pkt->length - pkt->savedpos, &b);

    if (j < 0)
	return NULL;

    pkt->savedpos += j;
    return b;
}
