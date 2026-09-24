GF_Err tx3g_dump(GF_Box *a, FILE * trace)
{
	GF_Tx3gSampleEntryBox *p = (GF_Tx3gSampleEntryBox *)a;
	gf_isom_box_dump_start(a, "Tx3gSampleEntryBox", trace);
	fprintf(trace, "dataReferenceIndex=\"%d\" displayFlags=\"%x\" horizontal-justification=\"%d\" vertical-justification=\"%d\" ",
	        p->dataReferenceIndex, p->displayFlags, p->horizontal_justification, p->vertical_justification);

	tx3g_dump_rgba8(trace, "backgroundColor", p->back_color);
	fprintf(trace, ">\n");
	fprintf(trace, "<DefaultBox>\n");
	tx3g_dump_box(trace, &p->default_box);
	gf_isom_box_dump_done("DefaultBox", a, trace);
	fprintf(trace, "<DefaultStyle>\n");
	tx3g_dump_style(trace, &p->default_style);
	fprintf(trace, "</DefaultStyle>\n");
	if (p->size) {
		gf_isom_box_dump_ex(p->font_table, trace, GF_ISOM_BOX_TYPE_FTAB);
	}
	gf_isom_box_dump_done("Tx3gSampleEntryBox", a, trace);
	return GF_OK;
}
