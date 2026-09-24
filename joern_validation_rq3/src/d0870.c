GF_Err stsd_Write(GF_Box *s, GF_BitStream *bs)
{
	GF_Err e;
	u32 nb_entries;
	GF_SampleDescriptionBox *ptr = (GF_SampleDescriptionBox *)s;

	e = gf_isom_full_box_write(s, bs);
	if (e) return e;
	nb_entries = gf_list_count(ptr->other_boxes);
	gf_bs_write_u32(bs, nb_entries);
	return GF_OK;
}
