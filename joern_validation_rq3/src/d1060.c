static int sctp_getsockopt_sctp_status(struct sock *sk, int len,
				       char __user *optval,
				       int __user *optlen)
{
	struct sctp_status status;
	struct sctp_association *asoc = NULL;
	struct sctp_transport *transport;
	sctp_assoc_t associd;
	int retval = 0;

	if (len < sizeof(status)) {
		retval = -EINVAL;
		goto out;
	}

	len = sizeof(status);
	if (copy_from_user(&status, optval, len)) {
		retval = -EFAULT;
		goto out;
	}

	associd = status.sstat_assoc_id;
	asoc = sctp_id2assoc(sk, associd);
	if (!asoc) {
		retval = -EINVAL;
		goto out;
	}

	transport = asoc->peer.primary_path;

	status.sstat_assoc_id = sctp_assoc2id(asoc);
	status.sstat_state = sctp_assoc_to_state(asoc);
	status.sstat_rwnd =  asoc->peer.rwnd;
	status.sstat_unackdata = asoc->unack_data;

	status.sstat_penddata = sctp_tsnmap_pending(&asoc->peer.tsn_map);
	status.sstat_instrms = asoc->stream.incnt;
	status.sstat_outstrms = asoc->stream.outcnt;
	status.sstat_fragmentation_point = asoc->frag_point;
	status.sstat_primary.spinfo_assoc_id = sctp_assoc2id(transport->asoc);
	memcpy(&status.sstat_primary.spinfo_address, &transport->ipaddr,
			transport->af_specific->sockaddr_len);
	/* Map ipv4 address into v4-mapped-on-v6 address.  */
	sctp_get_pf_specific(sk->sk_family)->addr_to_user(sctp_sk(sk),
		(union sctp_addr *)&status.sstat_primary.spinfo_address);
	status.sstat_primary.spinfo_state = transport->state;
	status.sstat_primary.spinfo_cwnd = transport->cwnd;
	status.sstat_primary.spinfo_srtt = transport->srtt;
	status.sstat_primary.spinfo_rto = jiffies_to_msecs(transport->rto);
	status.sstat_primary.spinfo_mtu = transport->pathmtu;

	if (status.sstat_primary.spinfo_state == SCTP_UNKNOWN)
		status.sstat_primary.spinfo_state = SCTP_ACTIVE;

	if (put_user(len, optlen)) {
		retval = -EFAULT;
		goto out;
	}

	pr_debug("%s: len:%d, state:%d, rwnd:%d, assoc_id:%d\n",
		 __func__, len, status.sstat_state, status.sstat_rwnd,
		 status.sstat_assoc_id);

	if (copy_to_user(optval, &status, len)) {
		retval = -EFAULT;
		goto out;
	}

out:
	return retval;
}
