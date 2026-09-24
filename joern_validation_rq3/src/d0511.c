static int phar_test_compression(void *pDest, void *argument TSRMLS_DC) /* {{{ */
{
	phar_entry_info *entry = (phar_entry_info *)pDest;

	if (entry->is_deleted) {
		return ZEND_HASH_APPLY_KEEP;
	}

	if (!PHAR_G(has_bz2)) {
		if (entry->flags & PHAR_ENT_COMPRESSED_BZ2) {
			*(int *) argument = 0;
		}
	}

	if (!PHAR_G(has_zlib)) {
		if (entry->flags & PHAR_ENT_COMPRESSED_GZ) {
			*(int *) argument = 0;
		}
	}

	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */
