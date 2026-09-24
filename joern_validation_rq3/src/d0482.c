dump_email(FILE *fp, void *data)
{
	char *addr = data;
	conf_write(fp, " Email notification = %s", addr);
}
