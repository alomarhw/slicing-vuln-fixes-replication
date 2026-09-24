static void mipspmu_event_update(struct perf_event *event,
			struct hw_perf_event *hwc,
			int idx)
{
	struct cpu_hw_events *cpuc = &__get_cpu_var(cpu_hw_events);
	unsigned long flags;
	int shift = 64 - TOTAL_BITS;
	s64 prev_raw_count, new_raw_count;
	u64 delta;

again:
	prev_raw_count = local64_read(&hwc->prev_count);
	local_irq_save(flags);
	/* Make the counter value be a "real" one. */
	new_raw_count = mipspmu->read_counter(idx);
	if (new_raw_count & (test_bit(idx, cpuc->msbs) << HIGHEST_BIT)) {
		new_raw_count &= VALID_COUNT;
		clear_bit(idx, cpuc->msbs);
	} else
		new_raw_count |= (test_bit(idx, cpuc->msbs) << HIGHEST_BIT);
	local_irq_restore(flags);

	if (local64_cmpxchg(&hwc->prev_count, prev_raw_count,
				new_raw_count) != prev_raw_count)
		goto again;

	delta = (new_raw_count << shift) - (prev_raw_count << shift);
	delta >>= shift;

	local64_add(delta, &event->count);
	local64_sub(delta, &hwc->period_left);

	return;
}
