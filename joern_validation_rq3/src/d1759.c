static u32 read_pmc(int idx)
{
	u64 val;

	read_pic(val);
	if (idx == PIC_UPPER_INDEX)
		val >>= 32;

	return val & 0xffffffff;
}
