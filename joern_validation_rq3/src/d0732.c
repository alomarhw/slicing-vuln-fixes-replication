_zip_open(zip_source_t *src, unsigned int flags, zip_error_t *error)
{
    zip_t *za;
    zip_cdir_t *cdir;
    struct zip_stat st;
    zip_uint64_t len, idx;

    zip_stat_init(&st);
    if (zip_source_stat(src, &st) < 0) {
	_zip_error_set_from_source(error, src);
	return NULL;
    }
    if ((st.valid & ZIP_STAT_SIZE) == 0) {
	zip_error_set(error, ZIP_ER_SEEK, EOPNOTSUPP);
	return NULL;
    }
    len = st.size;

    /* treat empty files as empty archives */
    if (len == 0) {
	if ((za=_zip_allocate_new(src, flags, error)) == NULL) {
	    zip_source_free(src);
	    return NULL;
	}

	return za;
    }

    if ((za=_zip_allocate_new(src, flags, error)) == NULL) {
        return NULL;
    }

    if ((cdir = _zip_find_central_dir(za, len)) == NULL) {
        _zip_error_copy(error, &za->error);
	/* keep src so discard does not get rid of it */
	zip_source_keep(src);
	zip_discard(za);
	return NULL;
    }

    za->entry = cdir->entry;
    za->nentry = cdir->nentry;
    za->nentry_alloc = cdir->nentry_alloc;
    za->comment_orig = cdir->comment;

    free(cdir);

    _zip_hash_reserve_capacity(za->names, za->nentry, &za->error);
    
    for (idx = 0; idx < za->nentry; idx++) {
	const zip_uint8_t *name = _zip_string_get(za->entry[idx].orig->filename, NULL, 0, error);
	if (name == NULL) {
		/* keep src so discard does not get rid of it */
		zip_source_keep(src);
		zip_discard(za);
		return NULL;
	}

	if (_zip_hash_add(za->names, name, idx, ZIP_FL_UNCHANGED, &za->error) == false) {
	    if (za->error.zip_err != ZIP_ER_EXISTS || (flags & ZIP_CHECKCONS)) {
		_zip_error_copy(error, &za->error);
		/* keep src so discard does not get rid of it */
		zip_source_keep(src);
		zip_discard(za);
		return NULL;
	    }
	}
    }

    za->ch_flags = za->flags;

    return za;
}
