free_decompression(struct archive_read *a, struct _7zip *zip)
{
	int r = ARCHIVE_OK;

#if !defined(HAVE_ZLIB_H) &&\
	!(defined(HAVE_BZLIB_H) && defined(BZ_CONFIG_ERROR))
	(void)a;/* UNUSED */
#endif
#ifdef HAVE_LZMA_H
	if (zip->lzstream_valid)
		lzma_end(&(zip->lzstream));
#endif
#if defined(HAVE_BZLIB_H) && defined(BZ_CONFIG_ERROR)
	if (zip->bzstream_valid) {
		if (BZ2_bzDecompressEnd(&(zip->bzstream)) != BZ_OK) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_MISC,
			    "Failed to clean up bzip2 decompressor");
			r = ARCHIVE_FATAL;
		}
		zip->bzstream_valid = 0;
	}
#endif
#ifdef HAVE_ZLIB_H
	if (zip->stream_valid) {
		if (inflateEnd(&(zip->stream)) != Z_OK) {
			archive_set_error(&a->archive,
			    ARCHIVE_ERRNO_MISC,
			    "Failed to clean up zlib decompressor");
			r = ARCHIVE_FATAL;
		}
		zip->stream_valid = 0;
	}
#endif
	if (zip->ppmd7_valid) {
		__archive_ppmd7_functions.Ppmd7_Free(
			&zip->ppmd7_context);
		zip->ppmd7_valid = 0;
	}
	return (r);
}
