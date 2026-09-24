GF_Err DTE_Dump(GF_List *dte, FILE * trace)
{
	GF_GenericDTE *p;
	GF_ImmediateDTE *i_p;
	GF_SampleDTE *s_p;
	GF_StreamDescDTE *sd_p;
	u32 i, count;

	count = gf_list_count(dte);
	for (i=0; i<count; i++) {
		p = (GF_GenericDTE *)gf_list_get(dte, i);
		switch (p->source) {
		case 0:
			fprintf(trace, "<EmptyDataEntry/>\n");
			break;
		case 1:
			i_p = (GF_ImmediateDTE *) p;
			fprintf(trace, "<ImmediateDataEntry DataSize=\"%d\"/>\n", i_p->dataLength);
			break;
		case 2:
			s_p = (GF_SampleDTE *) p;
			fprintf(trace, "<SampleDataEntry DataSize=\"%d\" SampleOffset=\"%d\" SampleNumber=\"%d\" TrackReference=\"%d\"/>\n",
			        s_p->dataLength, s_p->byteOffset, s_p->sampleNumber, s_p->trackRefIndex);
			break;
		case 3:
			sd_p = (GF_StreamDescDTE *) p;
			fprintf(trace, "<SampleDescriptionEntry DataSize=\"%d\" DescriptionOffset=\"%d\" StreamDescriptionindex=\"%d\" TrackReference=\"%d\"/>\n",
			        sd_p->dataLength, sd_p->byteOffset, sd_p->streamDescIndex, sd_p->trackRefIndex);
			break;
		default:
			fprintf(trace, "<UnknownTableEntry/>\n");
			break;
		}
	}
	return GF_OK;
}
