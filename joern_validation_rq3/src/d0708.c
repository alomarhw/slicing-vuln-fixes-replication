ZEND_API int zend_ts_hash_compare(TsHashTable *ht1, TsHashTable *ht2, compare_func_t compar, zend_bool ordered)
{
	int retval;

	begin_read(ht1);
	begin_read(ht2);
	retval = zend_hash_compare(TS_HASH(ht1), TS_HASH(ht2), compar, ordered);
	end_read(ht2);
	end_read(ht1);

	return retval;
}
