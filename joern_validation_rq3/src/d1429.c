 static int sfile_write(jas_stream_obj_t *obj, char *buf, int cnt)
 {
 	FILE *fp;
	size_t n;
	JAS_DBGLOG(100, ("sfile_write(%p, %p, %d)\n", obj, buf, cnt));
	fp = JAS_CAST(FILE *, obj);
	n = fwrite(buf, 1, cnt, fp);
 	return (n != JAS_CAST(size_t, cnt)) ? (-1) : cnt;
 }
