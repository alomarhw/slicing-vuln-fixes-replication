void dinf_del(GF_Box *s)
{
	GF_DataInformationBox *ptr = (GF_DataInformationBox *)s;
	if (ptr == NULL) return;
	gf_isom_box_del((GF_Box *)ptr->dref);
	gf_free(ptr);
}
