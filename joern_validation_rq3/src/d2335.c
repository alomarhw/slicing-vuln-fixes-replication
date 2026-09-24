PHP_MSHUTDOWN_FUNCTION(mb_regex)
{
	onig_end();
	return SUCCESS;
}
