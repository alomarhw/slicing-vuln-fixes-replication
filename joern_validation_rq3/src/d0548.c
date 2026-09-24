void php_gd_error_method(int type, const char *format, va_list args)
{
	TSRMLS_FETCH();

	php_verror(NULL, "", type, format, args TSRMLS_CC);
}
