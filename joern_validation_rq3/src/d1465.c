PHP_METHOD(PharFileInfo, getContent)
{
	char *error;
	php_stream *fp;
	phar_entry_info *link;
	zend_string *str;

	PHAR_ENTRY_OBJECT();

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (entry_obj->entry->is_dir) {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Phar error: Cannot retrieve contents, \"%s\" in phar \"%s\" is a directory", entry_obj->entry->filename, entry_obj->entry->phar->fname);
		return;
	}

	link = phar_get_link_source(entry_obj->entry);

	if (!link) {
		link = entry_obj->entry;
	}

	if (SUCCESS != phar_open_entry_fp(link, &error, 0)) {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Phar error: Cannot retrieve contents, \"%s\" in phar \"%s\": %s", entry_obj->entry->filename, entry_obj->entry->phar->fname, error);
		efree(error);
		return;
	}

	if (!(fp = phar_get_efp(link, 0))) {
		zend_throw_exception_ex(spl_ce_BadMethodCallException, 0,
			"Phar error: Cannot retrieve contents of \"%s\" in phar \"%s\"", entry_obj->entry->filename, entry_obj->entry->phar->fname);
		return;
	}

	phar_seek_efp(link, 0, SEEK_SET, 0, 0);
	str = php_stream_copy_to_mem(fp, link->uncompressed_filesize, 0);
	if (str) {
		RETURN_STR(str);
	} else {
		RETURN_EMPTY_STRING();
	}
}
