static void armv7pmu_start(void)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&pmu_lock, flags);
	/* Enable all counters */
	armv7_pmnc_write(armv7_pmnc_read() | ARMV7_PMNC_E);
	raw_spin_unlock_irqrestore(&pmu_lock, flags);
}
