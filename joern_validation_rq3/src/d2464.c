static int bgp_attr_encap(uint8_t type, struct peer *peer, /* IN */
			  bgp_size_t length, /* IN: attr's length field */
			  struct attr *attr, /* IN: caller already allocated */
			  uint8_t flag,      /* IN: attr's flags field */
			  uint8_t *startp)
{
	bgp_size_t total;
	uint16_t tunneltype = 0;

	total = length + (CHECK_FLAG(flag, BGP_ATTR_FLAG_EXTLEN) ? 4 : 3);

	if (!CHECK_FLAG(flag, BGP_ATTR_FLAG_TRANS)
	    || !CHECK_FLAG(flag, BGP_ATTR_FLAG_OPTIONAL)) {
		zlog_info(
			"Tunnel Encap attribute flag isn't optional and transitive %d",
			flag);
		bgp_notify_send_with_data(peer, BGP_NOTIFY_UPDATE_ERR,
					  BGP_NOTIFY_UPDATE_ATTR_FLAG_ERR,
					  startp, total);
		return -1;
	}

	if (BGP_ATTR_ENCAP == type) {
		/* read outer TLV type and length */
		uint16_t tlv_length;

		if (length < 4) {
			zlog_info(
				"Tunnel Encap attribute not long enough to contain outer T,L");
			bgp_notify_send_with_data(
				peer, BGP_NOTIFY_UPDATE_ERR,
				BGP_NOTIFY_UPDATE_OPT_ATTR_ERR, startp, total);
			return -1;
		}
		tunneltype = stream_getw(BGP_INPUT(peer));
		tlv_length = stream_getw(BGP_INPUT(peer));
		length -= 4;

		if (tlv_length != length) {
			zlog_info("%s: tlv_length(%d) != length(%d)", __func__,
				  tlv_length, length);
		}
	}

	while (length >= 4) {
		uint16_t subtype = 0;
		uint16_t sublength = 0;
		struct bgp_attr_encap_subtlv *tlv;

		if (BGP_ATTR_ENCAP == type) {
			subtype = stream_getc(BGP_INPUT(peer));
			sublength = stream_getc(BGP_INPUT(peer));
			length -= 2;
#if ENABLE_BGP_VNC
		} else {
			subtype = stream_getw(BGP_INPUT(peer));
			sublength = stream_getw(BGP_INPUT(peer));
			length -= 4;
#endif
		}

		if (sublength > length) {
			zlog_info(
				"Tunnel Encap attribute sub-tlv length %d exceeds remaining length %d",
				sublength, length);
			bgp_notify_send_with_data(
				peer, BGP_NOTIFY_UPDATE_ERR,
				BGP_NOTIFY_UPDATE_OPT_ATTR_ERR, startp, total);
			return -1;
		}

		/* alloc and copy sub-tlv */
		/* TBD make sure these are freed when attributes are released */
		tlv = XCALLOC(MTYPE_ENCAP_TLV,
			      sizeof(struct bgp_attr_encap_subtlv) + sublength);
		tlv->type = subtype;
		tlv->length = sublength;
		stream_get(tlv->value, peer->curr, sublength);
		length -= sublength;

		/* attach tlv to encap chain */
		if (BGP_ATTR_ENCAP == type) {
			struct bgp_attr_encap_subtlv *stlv_last;
			for (stlv_last = attr->encap_subtlvs;
			     stlv_last && stlv_last->next;
			     stlv_last = stlv_last->next)
				;
			if (stlv_last) {
				stlv_last->next = tlv;
			} else {
				attr->encap_subtlvs = tlv;
			}
#if ENABLE_BGP_VNC
		} else {
			struct bgp_attr_encap_subtlv *stlv_last;
			for (stlv_last = attr->vnc_subtlvs;
			     stlv_last && stlv_last->next;
			     stlv_last = stlv_last->next)
				;
			if (stlv_last) {
				stlv_last->next = tlv;
			} else {
				attr->vnc_subtlvs = tlv;
			}
#endif
		}
	}

	if (BGP_ATTR_ENCAP == type) {
		attr->encap_tunneltype = tunneltype;
	}

	if (length) {
		/* spurious leftover data */
		zlog_info(
			"Tunnel Encap attribute length is bad: %d leftover octets",
			length);
		bgp_notify_send_with_data(peer, BGP_NOTIFY_UPDATE_ERR,
					  BGP_NOTIFY_UPDATE_OPT_ATTR_ERR,
					  startp, total);
		return -1;
	}

	return 0;
}
