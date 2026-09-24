static char *get_path(const char *url, const char *www_dir)
{
	const char *p;
	char *res;

	if (!strlen(url) || !strcmp(url, ".") || !strcmp(url, "/"))
		p = "/index.html";
	else
		p = url;

	res = malloc(strlen(www_dir)+strlen(p)+1);

	strcpy(res, www_dir);
	strcat(res, p);

	return res;
}
