static zend_object *phar_convert_to_other(phar_archive_data *source, int convert, char *ext, php_uint32 flags) /* {{{ */
{
	phar_archive_data *phar;
	phar_entry_info *entry, newentry;
	zend_object *ret;

	/* invalidate phar cache */
	PHAR_G(last_phar) = NULL;
	PHAR_G(last_phar_name) = PHAR_G(last_alias) = NULL;

	phar = (phar_archive_data *) ecalloc(1, sizeof(phar_archive_data));
	/* set whole-archive compression and type from parameter */
	phar->flags = flags;
	phar->is_data = source->is_data;

	switch (convert) {
		case PHAR_FORMAT_TAR:
			phar->is_tar = 1;
			break;
		case PHAR_FORMAT_ZIP:
			phar->is_zip = 1;
			break;
		default:
			phar->is_data = 0;
			break;
	}

	zend_hash_init(&(phar->manifest), sizeof(phar_entry_info),
		zend_get_hash_value, destroy_phar_manifest_entry, 0);
	zend_hash_init(&phar->mounted_dirs, sizeof(char *),
		zend_get_hash_value, NULL, 0);
	zend_hash_init(&phar->virtual_dirs, sizeof(char *),
		zend_get_hash_value, NULL, 0);

	phar->fp = php_stream_fopen_tmpfile();
	if (phar->fp == NULL) {
		zend_throw_exception_ex(phar_ce_PharException, 0, "unable to create temporary file");
		return NULL;
	}
	phar->fname = source->fname;
	phar->fname_len = source->fname_len;
	phar->is_temporary_alias = source->is_temporary_alias;
	phar->alias = source->alias;

	if (Z_TYPE(source->metadata) != IS_UNDEF) {
		ZVAL_DUP(&phar->metadata, &source->metadata);
		phar->metadata_len = 0;
	}

	/* first copy each file's uncompressed contents to a temporary file and set per-file flags */
	ZEND_HASH_FOREACH_PTR(&source->manifest, entry) {

		newentry = *entry;

		if (newentry.link) {
			newentry.link = estrdup(newentry.link);
			goto no_copy;
		}

		if (newentry.tmp) {
			newentry.tmp = estrdup(newentry.tmp);
			goto no_copy;
		}

		newentry.metadata_str.s = NULL;

		if (FAILURE == phar_copy_file_contents(&newentry, phar->fp)) {
			zend_hash_destroy(&(phar->manifest));
			php_stream_close(phar->fp);
			efree(phar);
			/* exception already thrown */
			return NULL;
		}
no_copy:
		newentry.filename = estrndup(newentry.filename, newentry.filename_len);

		if (Z_TYPE(newentry.metadata) != IS_UNDEF) {
			zval_copy_ctor(&newentry.metadata);
			newentry.metadata_str.s = NULL;
		}

		newentry.is_zip = phar->is_zip;
		newentry.is_tar = phar->is_tar;

		if (newentry.is_tar) {
			newentry.tar_type = (entry->is_dir ? TAR_DIR : TAR_FILE);
		}

		newentry.is_modified = 1;
		newentry.phar = phar;
		newentry.old_flags = newentry.flags & ~PHAR_ENT_COMPRESSION_MASK; /* remove compression from old_flags */
		phar_set_inode(&newentry);
		zend_hash_str_add_mem(&(phar->manifest), newentry.filename, newentry.filename_len, (void*)&newentry, sizeof(phar_entry_info));
		phar_add_virtual_dirs(phar, newentry.filename, newentry.filename_len);
	} ZEND_HASH_FOREACH_END();

	if ((ret = phar_rename_archive(&phar, ext, 0))) {
		return ret;
	} else {
		if(phar != NULL) {
			zend_hash_destroy(&(phar->manifest));
			zend_hash_destroy(&(phar->mounted_dirs));
			zend_hash_destroy(&(phar->virtual_dirs));
			if (phar->fp) {
				php_stream_close(phar->fp);
			}
			efree(phar->fname);
			efree(phar);
		}
		return NULL;
	}
}
/* }}} */
