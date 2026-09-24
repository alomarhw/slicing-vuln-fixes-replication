current_fixup(struct archive_write_disk *a, const char *pathname)
{
	if (a->current_fixup == NULL)
		a->current_fixup = new_fixup(a, pathname);
	return (a->current_fixup);
}
