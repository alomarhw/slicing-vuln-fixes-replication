ZEND_API zval *zend_ts_hash_str_find(TsHashTable *ht, const char *key, size_t len)
{
	zval *retval;

	begin_read(ht);
	retval = zend_hash_str_find(TS_HASH(ht), key, len);
	end_read(ht);

	return retval;
}
