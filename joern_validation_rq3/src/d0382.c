PHP_FUNCTION(htmlspecialchars_decode)
{
	char *str;
	int str_len;
	size_t new_len = 0;
	long quote_style = ENT_COMPAT;
	char *replaced;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &str, &str_len, &quote_style) == FAILURE) {
		return;
	}

	replaced = php_unescape_html_entities(str, str_len, &new_len, 0 /*!all*/, quote_style, NULL TSRMLS_CC);
	if (replaced) {
		RETURN_STRINGL(replaced, (int)new_len, 0);
	}
	RETURN_FALSE;
}
