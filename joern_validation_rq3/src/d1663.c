GF_Err akey_dump(GF_Box *a, FILE * trace)
{
	GF_AdobeKeyInfoBox *ptr = (GF_AdobeKeyInfoBox *)a;
	if (!a) return GF_BAD_PARAM;
	gf_isom_box_dump_start(a, "AdobeKeyInfoBox", trace);
	fprintf(trace, ">\n");
	if (ptr->params) gf_isom_box_dump((GF_Box *)ptr->params, trace);
	gf_isom_box_dump_done("AdobeKeyInfoBox", a, trace);
	return GF_OK;
}
