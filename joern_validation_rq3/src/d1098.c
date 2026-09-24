print_nsp(netdissect_options *ndo,
          const u_char *nspp, u_int nsplen)
{
	const struct nsphdr *nsphp = (const struct nsphdr *)nspp;
	int dst, src, flags;

	if (nsplen < sizeof(struct nsphdr))
		goto trunc;
	ND_TCHECK(*nsphp);
	flags = EXTRACT_LE_8BITS(nsphp->nh_flags);
	dst = EXTRACT_LE_16BITS(nsphp->nh_dst);
	src = EXTRACT_LE_16BITS(nsphp->nh_src);

	switch (flags & NSP_TYPEMASK) {
	case MFT_DATA:
	    switch (flags & NSP_SUBMASK) {
	    case MFS_BOM:
	    case MFS_MOM:
	    case MFS_EOM:
	    case MFS_BOM+MFS_EOM:
		ND_PRINT((ndo, "data %d>%d ", src, dst));
		{
		    const struct seghdr *shp = (const struct seghdr *)nspp;
		    int ack;
		    u_int data_off = sizeof(struct minseghdr);

		    if (nsplen < data_off)
			goto trunc;
		    ND_TCHECK(shp->sh_seq[0]);
		    ack = EXTRACT_LE_16BITS(shp->sh_seq[0]);
		    if (ack & SGQ_ACK) {	/* acknum field */
			if ((ack & SGQ_NAK) == SGQ_NAK)
			    ND_PRINT((ndo, "nak %d ", ack & SGQ_MASK));
			else
			    ND_PRINT((ndo, "ack %d ", ack & SGQ_MASK));
			data_off += sizeof(short);
			if (nsplen < data_off)
			    goto trunc;
			ND_TCHECK(shp->sh_seq[1]);
		        ack = EXTRACT_LE_16BITS(shp->sh_seq[1]);
			if (ack & SGQ_OACK) {	/* ackoth field */
			    if ((ack & SGQ_ONAK) == SGQ_ONAK)
				ND_PRINT((ndo, "onak %d ", ack & SGQ_MASK));
			    else
				ND_PRINT((ndo, "oack %d ", ack & SGQ_MASK));
			    data_off += sizeof(short);
			    if (nsplen < data_off)
				goto trunc;
			    ND_TCHECK(shp->sh_seq[2]);
			    ack = EXTRACT_LE_16BITS(shp->sh_seq[2]);
			}
		    }
		    ND_PRINT((ndo, "seg %d ", ack & SGQ_MASK));
		}
		break;
	    case MFS_ILS+MFS_INT:
		ND_PRINT((ndo, "intr "));
		{
		    const struct seghdr *shp = (const struct seghdr *)nspp;
		    int ack;
		    u_int data_off = sizeof(struct minseghdr);

		    if (nsplen < data_off)
			goto trunc;
		    ND_TCHECK(shp->sh_seq[0]);
		    ack = EXTRACT_LE_16BITS(shp->sh_seq[0]);
		    if (ack & SGQ_ACK) {	/* acknum field */
			if ((ack & SGQ_NAK) == SGQ_NAK)
			    ND_PRINT((ndo, "nak %d ", ack & SGQ_MASK));
			else
			    ND_PRINT((ndo, "ack %d ", ack & SGQ_MASK));
			data_off += sizeof(short);
			if (nsplen < data_off)
			    goto trunc;
			ND_TCHECK(shp->sh_seq[1]);
		        ack = EXTRACT_LE_16BITS(shp->sh_seq[1]);
			if (ack & SGQ_OACK) {	/* ackdat field */
			    if ((ack & SGQ_ONAK) == SGQ_ONAK)
				ND_PRINT((ndo, "nakdat %d ", ack & SGQ_MASK));
			    else
				ND_PRINT((ndo, "ackdat %d ", ack & SGQ_MASK));
			    data_off += sizeof(short);
			    if (nsplen < data_off)
				goto trunc;
			    ND_TCHECK(shp->sh_seq[2]);
			    ack = EXTRACT_LE_16BITS(shp->sh_seq[2]);
			}
		    }
		    ND_PRINT((ndo, "seg %d ", ack & SGQ_MASK));
		}
		break;
	    case MFS_ILS:
		ND_PRINT((ndo, "link-service %d>%d ", src, dst));
		{
		    const struct seghdr *shp = (const struct seghdr *)nspp;
		    const struct lsmsg *lsmp =
			(const struct lsmsg *)&(nspp[sizeof(struct seghdr)]);
		    int ack;
		    int lsflags, fcval;

		    if (nsplen < sizeof(struct seghdr) + sizeof(struct lsmsg))
			goto trunc;
		    ND_TCHECK(shp->sh_seq[0]);
		    ack = EXTRACT_LE_16BITS(shp->sh_seq[0]);
		    if (ack & SGQ_ACK) {	/* acknum field */
			if ((ack & SGQ_NAK) == SGQ_NAK)
			    ND_PRINT((ndo, "nak %d ", ack & SGQ_MASK));
			else
			    ND_PRINT((ndo, "ack %d ", ack & SGQ_MASK));
			ND_TCHECK(shp->sh_seq[1]);
		        ack = EXTRACT_LE_16BITS(shp->sh_seq[1]);
			if (ack & SGQ_OACK) {	/* ackdat field */
			    if ((ack & SGQ_ONAK) == SGQ_ONAK)
				ND_PRINT((ndo, "nakdat %d ", ack & SGQ_MASK));
			    else
				ND_PRINT((ndo, "ackdat %d ", ack & SGQ_MASK));
			    ND_TCHECK(shp->sh_seq[2]);
			    ack = EXTRACT_LE_16BITS(shp->sh_seq[2]);
			}
		    }
		    ND_PRINT((ndo, "seg %d ", ack & SGQ_MASK));
		    ND_TCHECK(*lsmp);
		    lsflags = EXTRACT_LE_8BITS(lsmp->ls_lsflags);
		    fcval = EXTRACT_LE_8BITS(lsmp->ls_fcval);
		    switch (lsflags & LSI_MASK) {
		    case LSI_DATA:
			ND_PRINT((ndo, "dat seg count %d ", fcval));
			switch (lsflags & LSM_MASK) {
			case LSM_NOCHANGE:
			    break;
			case LSM_DONOTSEND:
			    ND_PRINT((ndo, "donotsend-data "));
			    break;
			case LSM_SEND:
			    ND_PRINT((ndo, "send-data "));
			    break;
			default:
			    ND_PRINT((ndo, "reserved-fcmod? %x", lsflags));
			    break;
			}
			break;
		    case LSI_INTR:
			ND_PRINT((ndo, "intr req count %d ", fcval));
			break;
		    default:
			ND_PRINT((ndo, "reserved-fcval-int? %x", lsflags));
			break;
		    }
		}
		break;
	    default:
		ND_PRINT((ndo, "reserved-subtype? %x %d > %d", flags, src, dst));
		break;
	    }
	    break;
	case MFT_ACK:
	    switch (flags & NSP_SUBMASK) {
	    case MFS_DACK:
		ND_PRINT((ndo, "data-ack %d>%d ", src, dst));
		{
		    const struct ackmsg *amp = (const struct ackmsg *)nspp;
		    int ack;

		    if (nsplen < sizeof(struct ackmsg))
			goto trunc;
		    ND_TCHECK(*amp);
		    ack = EXTRACT_LE_16BITS(amp->ak_acknum[0]);
		    if (ack & SGQ_ACK) {	/* acknum field */
			if ((ack & SGQ_NAK) == SGQ_NAK)
			    ND_PRINT((ndo, "nak %d ", ack & SGQ_MASK));
			else
			    ND_PRINT((ndo, "ack %d ", ack & SGQ_MASK));
		        ack = EXTRACT_LE_16BITS(amp->ak_acknum[1]);
			if (ack & SGQ_OACK) {	/* ackoth field */
			    if ((ack & SGQ_ONAK) == SGQ_ONAK)
				ND_PRINT((ndo, "onak %d ", ack & SGQ_MASK));
			    else
				ND_PRINT((ndo, "oack %d ", ack & SGQ_MASK));
			}
		    }
		}
		break;
	    case MFS_IACK:
		ND_PRINT((ndo, "ils-ack %d>%d ", src, dst));
		{
		    const struct ackmsg *amp = (const struct ackmsg *)nspp;
		    int ack;

		    if (nsplen < sizeof(struct ackmsg))
			goto trunc;
		    ND_TCHECK(*amp);
		    ack = EXTRACT_LE_16BITS(amp->ak_acknum[0]);
		    if (ack & SGQ_ACK) {	/* acknum field */
			if ((ack & SGQ_NAK) == SGQ_NAK)
			    ND_PRINT((ndo, "nak %d ", ack & SGQ_MASK));
			else
			    ND_PRINT((ndo, "ack %d ", ack & SGQ_MASK));
			ND_TCHECK(amp->ak_acknum[1]);
		        ack = EXTRACT_LE_16BITS(amp->ak_acknum[1]);
			if (ack & SGQ_OACK) {	/* ackdat field */
			    if ((ack & SGQ_ONAK) == SGQ_ONAK)
				ND_PRINT((ndo, "nakdat %d ", ack & SGQ_MASK));
			    else
				ND_PRINT((ndo, "ackdat %d ", ack & SGQ_MASK));
			}
		    }
		}
		break;
	    case MFS_CACK:
		ND_PRINT((ndo, "conn-ack %d", dst));
		break;
	    default:
		ND_PRINT((ndo, "reserved-acktype? %x %d > %d", flags, src, dst));
		break;
	    }
	    break;
	case MFT_CTL:
	    switch (flags & NSP_SUBMASK) {
	    case MFS_CI:
	    case MFS_RCI:
		if ((flags & NSP_SUBMASK) == MFS_CI)
		    ND_PRINT((ndo, "conn-initiate "));
		else
		    ND_PRINT((ndo, "retrans-conn-initiate "));
		ND_PRINT((ndo, "%d>%d ", src, dst));
		{
		    const struct cimsg *cimp = (const struct cimsg *)nspp;
		    int services, info, segsize;

		    if (nsplen < sizeof(struct cimsg))
			goto trunc;
		    ND_TCHECK(*cimp);
		    services = EXTRACT_LE_8BITS(cimp->ci_services);
		    info = EXTRACT_LE_8BITS(cimp->ci_info);
		    segsize = EXTRACT_LE_16BITS(cimp->ci_segsize);

		    switch (services & COS_MASK) {
		    case COS_NONE:
			break;
		    case COS_SEGMENT:
			ND_PRINT((ndo, "seg "));
			break;
		    case COS_MESSAGE:
			ND_PRINT((ndo, "msg "));
			break;
		    }
		    switch (info & COI_MASK) {
		    case COI_32:
			ND_PRINT((ndo, "ver 3.2 "));
			break;
		    case COI_31:
			ND_PRINT((ndo, "ver 3.1 "));
			break;
		    case COI_40:
			ND_PRINT((ndo, "ver 4.0 "));
			break;
		    case COI_41:
			ND_PRINT((ndo, "ver 4.1 "));
			break;
		    }
		    ND_PRINT((ndo, "segsize %d ", segsize));
		}
		break;
	    case MFS_CC:
		ND_PRINT((ndo, "conn-confirm %d>%d ", src, dst));
		{
		    const struct ccmsg *ccmp = (const struct ccmsg *)nspp;
		    int services, info;
		    u_int segsize, optlen;

		    if (nsplen < sizeof(struct ccmsg))
			goto trunc;
		    ND_TCHECK(*ccmp);
		    services = EXTRACT_LE_8BITS(ccmp->cc_services);
		    info = EXTRACT_LE_8BITS(ccmp->cc_info);
		    segsize = EXTRACT_LE_16BITS(ccmp->cc_segsize);
		    optlen = EXTRACT_LE_8BITS(ccmp->cc_optlen);

		    switch (services & COS_MASK) {
		    case COS_NONE:
			break;
		    case COS_SEGMENT:
			ND_PRINT((ndo, "seg "));
			break;
		    case COS_MESSAGE:
			ND_PRINT((ndo, "msg "));
			break;
		    }
		    switch (info & COI_MASK) {
		    case COI_32:
			ND_PRINT((ndo, "ver 3.2 "));
			break;
		    case COI_31:
			ND_PRINT((ndo, "ver 3.1 "));
			break;
		    case COI_40:
			ND_PRINT((ndo, "ver 4.0 "));
			break;
		    case COI_41:
			ND_PRINT((ndo, "ver 4.1 "));
			break;
		    }
		    ND_PRINT((ndo, "segsize %d ", segsize));
		    if (optlen) {
			ND_PRINT((ndo, "optlen %d ", optlen));
		    }
		}
		break;
	    case MFS_DI:
		ND_PRINT((ndo, "disconn-initiate %d>%d ", src, dst));
		{
		    const struct dimsg *dimp = (const struct dimsg *)nspp;
		    int reason;
		    u_int optlen;

		    if (nsplen < sizeof(struct dimsg))
			goto trunc;
		    ND_TCHECK(*dimp);
		    reason = EXTRACT_LE_16BITS(dimp->di_reason);
		    optlen = EXTRACT_LE_8BITS(dimp->di_optlen);

		    print_reason(ndo, reason);
		    if (optlen) {
			ND_PRINT((ndo, "optlen %d ", optlen));
		    }
		}
		break;
	    case MFS_DC:
		ND_PRINT((ndo, "disconn-confirm %d>%d ", src, dst));
		{
		    const struct dcmsg *dcmp = (const struct dcmsg *)nspp;
		    int reason;

		    ND_TCHECK(*dcmp);
		    reason = EXTRACT_LE_16BITS(dcmp->dc_reason);

		    print_reason(ndo, reason);
		}
		break;
	    default:
		ND_PRINT((ndo, "reserved-ctltype? %x %d > %d", flags, src, dst));
		break;
	    }
	    break;
	default:
	    ND_PRINT((ndo, "reserved-type? %x %d > %d", flags, src, dst));
	    break;
	}
	return (1);

trunc:
	return (0);
}
