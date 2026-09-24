struct sctp_chunk *sctp_make_datafrag_empty(struct sctp_association *asoc,
				       const struct sctp_sndrcvinfo *sinfo,
				       int data_len, __u8 flags, __u16 ssn)
{
	struct sctp_chunk *retval;
	struct sctp_datahdr dp;
	int chunk_len;

	/* We assign the TSN as LATE as possible, not here when
	 * creating the chunk.
	 */
	dp.tsn = 0;
	dp.stream = htons(sinfo->sinfo_stream);
	dp.ppid   = sinfo->sinfo_ppid;

	/* Set the flags for an unordered send.  */
	if (sinfo->sinfo_flags & SCTP_UNORDERED) {
		flags |= SCTP_DATA_UNORDERED;
		dp.ssn = 0;
	} else
		dp.ssn = htons(ssn);

	chunk_len = sizeof(dp) + data_len;
	retval = sctp_make_data(asoc, flags, chunk_len);
	if (!retval)
		goto nodata;

	retval->subh.data_hdr = sctp_addto_chunk(retval, sizeof(dp), &dp);
	memcpy(&retval->sinfo, sinfo, sizeof(struct sctp_sndrcvinfo));

nodata:
	return retval;
}
