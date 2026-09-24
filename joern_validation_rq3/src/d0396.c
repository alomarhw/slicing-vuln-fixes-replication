sctp_disposition_t sctp_sf_unk_chunk(struct net *net,
				     const struct sctp_endpoint *ep,
				     const struct sctp_association *asoc,
				     const sctp_subtype_t type,
				     void *arg,
				     sctp_cmd_seq_t *commands)
{
	struct sctp_chunk *unk_chunk = arg;
	struct sctp_chunk *err_chunk;
	sctp_chunkhdr_t *hdr;

	pr_debug("%s: processing unknown chunk id:%d\n", __func__, type.chunk);

	if (!sctp_vtag_verify(unk_chunk, asoc))
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);

	/* Make sure that the chunk has a valid length.
	 * Since we don't know the chunk type, we use a general
	 * chunkhdr structure to make a comparison.
	 */
	if (!sctp_chunk_length_valid(unk_chunk, sizeof(sctp_chunkhdr_t)))
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);

	switch (type.chunk & SCTP_CID_ACTION_MASK) {
	case SCTP_CID_ACTION_DISCARD:
		/* Discard the packet.  */
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
	case SCTP_CID_ACTION_DISCARD_ERR:
		/* Generate an ERROR chunk as response. */
		hdr = unk_chunk->chunk_hdr;
		err_chunk = sctp_make_op_error(asoc, unk_chunk,
					       SCTP_ERROR_UNKNOWN_CHUNK, hdr,
					       WORD_ROUND(ntohs(hdr->length)),
					       0);
		if (err_chunk) {
			sctp_add_cmd_sf(commands, SCTP_CMD_REPLY,
					SCTP_CHUNK(err_chunk));
		}

		/* Discard the packet.  */
		sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
		return SCTP_DISPOSITION_CONSUME;
	case SCTP_CID_ACTION_SKIP:
		/* Skip the chunk.  */
		return SCTP_DISPOSITION_DISCARD;
	case SCTP_CID_ACTION_SKIP_ERR:
		/* Generate an ERROR chunk as response. */
		hdr = unk_chunk->chunk_hdr;
		err_chunk = sctp_make_op_error(asoc, unk_chunk,
					       SCTP_ERROR_UNKNOWN_CHUNK, hdr,
					       WORD_ROUND(ntohs(hdr->length)),
					       0);
		if (err_chunk) {
			sctp_add_cmd_sf(commands, SCTP_CMD_REPLY,
					SCTP_CHUNK(err_chunk));
		}
		/* Skip the chunk.  */
		return SCTP_DISPOSITION_CONSUME;
	default:
		break;
	}

	return SCTP_DISPOSITION_DISCARD;
}
