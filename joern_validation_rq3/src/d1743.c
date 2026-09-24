move_reading_point(struct archive_read *a, uint64_t offset)
{
	struct xar *xar;

	xar = (struct xar *)(a->format->data);
	if (xar->offset - xar->h_base != offset) {
		/* Seek forward to the start of file contents. */
		int64_t step;

		step = offset - (xar->offset - xar->h_base);
		if (step > 0) {
			step = __archive_read_consume(a, step);
			if (step < 0)
				return ((int)step);
			xar->offset += step;
		} else {
			int64_t pos = __archive_read_seek(a, offset, SEEK_SET);
			if (pos == ARCHIVE_FAILED) {
				archive_set_error(&(a->archive),
				    ARCHIVE_ERRNO_MISC,
				    "Cannot seek.");
				return (ARCHIVE_FAILED);
			}
			xar->offset = pos;
		}
	}
	return (ARCHIVE_OK);
}
