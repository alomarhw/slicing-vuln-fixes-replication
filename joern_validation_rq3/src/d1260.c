static char *MakeNewMapValue(STRING2PTR value, const char *mapstr)
{
    char *ptr;
    char *newtitle = NULL;

    StrAllocCopy(newtitle, "[");
    StrAllocCat(newtitle, mapstr);	/* ISMAP or USEMAP */
    if (verbose_img && non_empty(value[HTML_IMG_SRC])) {
	StrAllocCat(newtitle, ":");
	ptr = strrchr(value[HTML_IMG_SRC], '/');
	if (!ptr) {
	    StrAllocCat(newtitle, value[HTML_IMG_SRC]);
	} else {
	    StrAllocCat(newtitle, ptr + 1);
	}
    }
    StrAllocCat(newtitle, "]");
    return newtitle;
}
