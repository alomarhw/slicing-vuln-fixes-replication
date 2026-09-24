static inline void apic_set_isr(int vec, struct kvm_lapic *apic)
{
	if (!__apic_test_and_set_vector(vec, apic->regs + APIC_ISR))
		++apic->isr_count;
	BUG_ON(apic->isr_count > MAX_APIC_VECTOR);
	/*
	 * ISR (in service register) bit is set when injecting an interrupt.
	 * The highest vector is injected. Thus the latest bit set matches
	 * the highest bit in ISR.
	 */
	apic->highest_isr_cache = vec;
}
