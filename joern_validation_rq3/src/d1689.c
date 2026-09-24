GF_Err tsel_dump(GF_Box *a, FILE * trace)
{
	u32 i;
	GF_TrackSelectionBox *ptr = (GF_TrackSelectionBox *)a;
	gf_isom_box_dump_start(a, "TrackSelectionBox", trace);

	fprintf(trace, "switchGroup=\"%d\" >\n", ptr->switchGroup);
	for (i=0; i<ptr->attributeListCount; i++) {
		fprintf(trace, "<TrackSelectionCriteria value=\"%s\"/>\n", gf_4cc_to_str(ptr->attributeList[i]) );
	}
	if (!ptr->size)
		fprintf(trace, "<TrackSelectionCriteria value=\"\"/>\n");

	gf_isom_box_dump_done("TrackSelectionBox", a, trace);
	return GF_OK;
}
