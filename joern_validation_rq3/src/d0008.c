static int p4_hw_config(struct perf_event *event)
{
	int cpu = get_cpu();
	int rc = 0;
	u32 escr, cccr;

	/*
	 * the reason we use cpu that early is that: if we get scheduled
	 * first time on the same cpu -- we will not need swap thread
	 * specific flags in config (and will save some cpu cycles)
	 */

	cccr = p4_default_cccr_conf(cpu);
	escr = p4_default_escr_conf(cpu, event->attr.exclude_kernel,
					 event->attr.exclude_user);
	event->hw.config = p4_config_pack_escr(escr) |
			   p4_config_pack_cccr(cccr);

	if (p4_ht_active() && p4_ht_thread(cpu))
		event->hw.config = p4_set_ht_bit(event->hw.config);

	if (event->attr.type == PERF_TYPE_RAW) {
		struct p4_event_bind *bind;
		unsigned int esel;
		/*
		 * Clear bits we reserve to be managed by kernel itself
		 * and never allowed from a user space
		 */
		 event->attr.config &= P4_CONFIG_MASK;

		rc = p4_validate_raw_event(event);
		if (rc)
			goto out;

		/*
		 * Note that for RAW events we allow user to use P4_CCCR_RESERVED
		 * bits since we keep additional info here (for cache events and etc)
		 */
		event->hw.config |= event->attr.config;
		bind = p4_config_get_bind(event->attr.config);
		if (!bind) {
			rc = -EINVAL;
			goto out;
		}
		esel = P4_OPCODE_ESEL(bind->opcode);
		event->hw.config |= p4_config_pack_cccr(P4_CCCR_ESEL(esel));
	}

	rc = x86_setup_perfctr(event);
out:
	put_cpu();
	return rc;
}
