static void fpm_children_cleanup(int which, void *arg) /* {{{ */
{
	free(last_faults);
}
/* }}} */
