GF_Err gen_sample_entry_Size(GF_Box *s)
{
	GF_SampleEntryBox *ptr = (GF_SampleEntryBox *)s;
	ptr->size += 8;
	return gf_isom_box_array_size(s, ptr->protections);
}
