static PHP_FUNCTION(xmlwriter_set_indent_string)
{
	php_xmlwriter_string_arg(INTERNAL_FUNCTION_PARAM_PASSTHRU, xmlTextWriterSetIndentString, NULL);
}
