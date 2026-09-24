static int emulate_swpX(unsigned int address, unsigned int *data,
			unsigned int type)
{
	unsigned int res = 0;

	if ((type != TYPE_SWPB) && (address & 0x3)) {
		/* SWP to unaligned address not permitted */
		pr_debug("SWP instruction on unaligned pointer!\n");
		return -EFAULT;
	}

	while (1) {
		unsigned long temp;

		/*
		 * Barrier required between accessing protected resource and
		 * releasing a lock for it. Legacy code might not have done
		 * this, and we cannot determine that this is not the case
		 * being emulated, so insert always.
		 */
		smp_mb();

		if (type == TYPE_SWPB)
			__user_swpb_asm(*data, address, res, temp);
		else
			__user_swp_asm(*data, address, res, temp);

		if (likely(res != -EAGAIN) || signal_pending(current))
			break;

		cond_resched();
	}

	if (res == 0) {
		/*
		 * Barrier also required between acquiring a lock for a
		 * protected resource and accessing the resource. Inserted for
		 * same reason as above.
		 */
		smp_mb();

		if (type == TYPE_SWPB)
			swpbcounter++;
		else
			swpcounter++;
	}

	return res;
}
