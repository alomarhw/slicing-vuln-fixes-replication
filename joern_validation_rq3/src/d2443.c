XcursorFilenameLoadImages (const char *file, int size)
{
    FILE	    *f;
    XcursorImages   *images;

    if (!file || size < 0)
        return NULL;

    f = fopen (file, "r");
    if (!f)
	return NULL;
    images = XcursorFileLoadImages (f, size);
    fclose (f);
    return images;
}
