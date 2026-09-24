check_fmt(struct magic_set *ms, struct magic *m)
{
	regex_t rx;
	int rc, rv = -1;

	if (strchr(m->desc, '%') == NULL)
		return 0;

	(void)setlocale(LC_CTYPE, "C");
	rc = regcomp(&rx, "%[-0-9\\.]*s", REG_EXTENDED|REG_NOSUB);
	if (rc) {
		char errmsg[512];
		(void)regerror(rc, &rx, errmsg, sizeof(errmsg));
		file_magerror(ms, "regex error %d, (%s)", rc, errmsg);
	} else {
		rc = regexec(&rx, m->desc, 0, 0, 0);
		regfree(&rx);
		rv = !rc;
	}
	(void)setlocale(LC_CTYPE, "");
	return rv;
}
