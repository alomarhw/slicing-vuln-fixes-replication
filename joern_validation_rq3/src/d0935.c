ZEND_API void ZEND_FASTCALL zend_hash_internal_pointer_end_ex(HashTable *ht, HashPosition *pos)
{
	uint32_t idx;

	IS_CONSISTENT(ht);
	HT_ASSERT(&ht->nInternalPointer != pos || GC_REFCOUNT(ht) == 1);

	idx = ht->nNumUsed;
	while (idx > 0) {
		idx--;
		if (Z_TYPE(ht->arData[idx].val) != IS_UNDEF) {
			*pos = idx;
			return;
		}
	}
	*pos = HT_INVALID_IDX;
}
