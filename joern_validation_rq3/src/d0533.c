GF_Err fiin_Size(GF_Box *s)
{
	GF_Err e;
	FDItemInformationBox *ptr = (FDItemInformationBox *)s;

	ptr->size+= 2;
	if (ptr->partition_entries) {
		e = gf_isom_box_array_size(s, ptr->partition_entries);
		if (e) return e;
	}
	if (ptr->session_info) {
		e = gf_isom_box_size((GF_Box *)ptr->session_info);
		if (e) return e;
		ptr->size += ptr->session_info->size;
	}
	if (ptr->group_id_to_name) {
		e = gf_isom_box_size((GF_Box *) ptr->group_id_to_name);
		if (e) return e;
		ptr->size += ptr->group_id_to_name->size;
	}

	return GF_OK;
}
