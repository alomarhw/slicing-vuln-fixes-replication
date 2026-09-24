GF_Err blnk_dump(GF_Box *a, FILE * trace)
{
	GF_TextBlinkBox*p = (GF_TextBlinkBox*)a;
	gf_isom_box_dump_start(a, "TextBlinkBox", trace);
	fprintf(trace, "start_charoffset=\"%d\" end_charoffset=\"%d\">\n", p->startcharoffset, p->endcharoffset);
	gf_isom_box_dump_done("TextBlinkBox", a, trace);
	return GF_OK;
}
