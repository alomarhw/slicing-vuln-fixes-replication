IW_IMPL(int) iw_max_color_to_bitdepth(unsigned int mc)
{
	unsigned int bd;

	for(bd=1;bd<=15;bd++) {
		if(mc < (1U<<bd)) return bd;
	}
	return 16;
}
