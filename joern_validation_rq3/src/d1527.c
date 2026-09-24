cdf_unpack_catalog(const cdf_header_t *h, const cdf_stream_t *sst,
    cdf_catalog_t **cat)
{
	size_t ss = sst->sst_dirlen < h->h_min_size_standard_stream ?
	    CDF_SHORT_SEC_SIZE(h) : CDF_SEC_SIZE(h);
	const char *b = CAST(const char *, sst->sst_tab);
	const char *eb = b + ss * sst->sst_len;
	size_t nr, i, k;
	cdf_catalog_entry_t *ce;
	uint16_t reclen;
	const uint16_t *np;

	for (nr = 0; b < eb; nr++) {
		memcpy(&reclen, b, sizeof(reclen));
		reclen = CDF_TOLE2(reclen);
		if (reclen == 0)
			break;
		b += reclen;
	}
	*cat = CAST(cdf_catalog_t *,
	    malloc(sizeof(cdf_catalog_t) + nr * sizeof(*ce)));
	(*cat)->cat_num = nr;
	ce = (*cat)->cat_e;
	b = CAST(const char *, sst->sst_tab);
	for (i = 0; i < nr; i++) {
		extract_catalog_field(ce_namlen, 0);
		extract_catalog_field(ce_num, 2);
		extract_catalog_field(ce_timestamp, 6);
		reclen = ce[i].ce_namlen;
		ce[i].ce_namlen =
		    sizeof(ce[i].ce_name) / sizeof(ce[i].ce_name[0]) - 1;
		if (ce[i].ce_namlen > reclen - 14)
			ce[i].ce_namlen = reclen - 14;
		np = CAST(const uint16_t *, (b + 16));
		for (k = 0; k < ce[i].ce_namlen; k++) {
			ce[i].ce_name[k] = np[k];
			CDF_TOLE2(ce[i].ce_name[k]);
		}
		ce[i].ce_name[ce[i].ce_namlen] = 0;
		b += reclen;
	}
	return 0;
}
