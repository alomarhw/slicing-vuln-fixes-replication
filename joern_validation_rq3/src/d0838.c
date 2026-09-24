int bgp_mp_unreach_parse(struct bgp_attr_parser_args *args,
			 struct bgp_nlri *mp_withdraw)
{
	struct stream *s;
	iana_afi_t pkt_afi;
	afi_t afi;
	iana_safi_t pkt_safi;
	safi_t safi;
	uint16_t withdraw_len;
	struct peer *const peer = args->peer;
	struct attr *const attr = args->attr;
	const bgp_size_t length = args->length;

	s = peer->curr;

#define BGP_MP_UNREACH_MIN_SIZE 3
	if ((length > STREAM_READABLE(s)) || (length < BGP_MP_UNREACH_MIN_SIZE))
		return BGP_ATTR_PARSE_ERROR_NOTIFYPLS;

	pkt_afi = stream_getw(s);
	pkt_safi = stream_getc(s);

	/* Convert AFI, SAFI to internal values, check. */
	if (bgp_map_afi_safi_iana2int(pkt_afi, pkt_safi, &afi, &safi)) {
		/* Log if AFI or SAFI is unrecognized. This is not an error
		 * unless
		 * the attribute is otherwise malformed.
		 */
		if (bgp_debug_update(peer, NULL, NULL, 0))
			zlog_debug(
				"%s: MP_UNREACH received AFI %u or SAFI %u is unrecognized",
				peer->host, pkt_afi, pkt_safi);
		return BGP_ATTR_PARSE_ERROR;
	}

	withdraw_len = length - BGP_MP_UNREACH_MIN_SIZE;

	mp_withdraw->afi = afi;
	mp_withdraw->safi = safi;
	mp_withdraw->nlri = stream_pnt(s);
	mp_withdraw->length = withdraw_len;

	stream_forward_getp(s, withdraw_len);

	attr->flag |= ATTR_FLAG_BIT(BGP_ATTR_MP_UNREACH_NLRI);

	return BGP_ATTR_PARSE_PROCEED;
}
