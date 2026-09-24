static gboolean toshiba_check_file_type(wtap *wth, int *err, gchar **err_info)
{
	char	buf[TOSHIBA_LINE_LENGTH];
	guint	i, reclen, level, line;
	char	byte;

	buf[TOSHIBA_LINE_LENGTH-1] = 0;

	for (line = 0; line < TOSHIBA_HEADER_LINES_TO_CHECK; line++) {
		if (file_gets(buf, TOSHIBA_LINE_LENGTH, wth->fh) == NULL) {
			/* EOF or error. */
			*err = file_error(wth->fh, err_info);
			return FALSE;
		}

		reclen = (guint) strlen(buf);
		if (reclen < TOSHIBA_HDR_MAGIC_SIZE) {
			continue;
		}

		level = 0;
		for (i = 0; i < reclen; i++) {
			byte = buf[i];
			if (byte == toshiba_hdr_magic[level]) {
				level++;
				if (level >= TOSHIBA_HDR_MAGIC_SIZE) {
					return TRUE;
				}
			}
			else {
				level = 0;
			}
		}
	}
	*err = 0;
	return FALSE;
}
