static ZIPARCHIVE_METHOD(getCommentName)
{
	struct zip *intern;
	zval *this = getThis();
	int name_len, idx;
	long flags = 0;
	int comment_len = 0;
	const char * comment;
	char *name;

	if (!this) {
		RETURN_FALSE;
	}

	ZIP_FROM_OBJECT(intern, this);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l",
			&name, &name_len, &flags) == FAILURE) {
		return;
	}
	if (name_len < 1) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Empty string as entry name");
		RETURN_FALSE;
	}

	idx = zip_name_locate(intern, name, 0);
	if (idx < 0) {
		RETURN_FALSE;
	}

	comment = zip_get_file_comment(intern, idx, &comment_len, (int)flags);
	RETURN_STRINGL((char *)comment, (long)comment_len, 1);
}
