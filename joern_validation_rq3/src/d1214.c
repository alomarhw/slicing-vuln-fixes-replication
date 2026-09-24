GF_Err trex_Write(GF_Box *s, GF_BitStream *bs)
{
	GF_Err e;
	GF_TrackExtendsBox *ptr = (GF_TrackExtendsBox *) s;
	if (!s) return GF_BAD_PARAM;
	e = gf_isom_full_box_write(s, bs);
	if (e) return e;

	gf_bs_write_u32(bs, ptr->trackID);
	gf_bs_write_u32(bs, ptr->def_sample_desc_index ? ptr->def_sample_desc_index : 1);
	gf_bs_write_u32(bs, ptr->def_sample_duration);
	gf_bs_write_u32(bs, ptr->def_sample_size);
	gf_bs_write_u32(bs, ptr->def_sample_flags);
	return GF_OK;
}
