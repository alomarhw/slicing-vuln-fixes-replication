static void dump_data(FILE *trace, char *data, u32 dataLength)
{
	u32 i;
	fprintf(trace, "data:application/octet-string,");
	for (i=0; i<dataLength; i++) {
		fprintf(trace, "%02X", (unsigned char) data[i]);
	}
}
