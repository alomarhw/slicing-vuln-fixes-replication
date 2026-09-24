PHP_FUNCTION(pg_unescape_bytea)
{
	char *from = NULL, *to = NULL, *tmp = NULL;
	size_t to_len;
	size_t from_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s",
							  &from, &from_len) == FAILURE) {
		return;
	}

#if HAVE_PQUNESCAPEBYTEA
	tmp = (char *)PQunescapeBytea((unsigned char*)from, &to_len);
	to = estrndup(tmp, to_len);
	PQfreemem(tmp);
#else
	to = (char *)php_pgsql_unescape_bytea((unsigned char*)from, &to_len);
#endif
	if (!to) {
		php_error_docref(NULL, E_WARNING,"Invalid parameter");
		RETURN_FALSE;
	}
	RETVAL_STRINGL(to, to_len);
	efree(to);
}
