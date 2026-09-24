static int intel_alt_er(int idx)
{
	if (!(x86_pmu.er_flags & ERF_HAS_RSP_1))
		return idx;

	if (idx == EXTRA_REG_RSP_0)
		return EXTRA_REG_RSP_1;

	if (idx == EXTRA_REG_RSP_1)
		return EXTRA_REG_RSP_0;

	return idx;
}
