int zend_shared_memdup_size(void *source, size_t size)
{
	void *old_p;

	if ((old_p = zend_hash_index_find_ptr(&xlat_table, (zend_ulong)source)) != NULL) {
		/* we already duplicated this pointer */
		return 0;
	}
	zend_shared_alloc_register_xlat_entry(source, source);
	return ZEND_ALIGNED_SIZE(size);
}
