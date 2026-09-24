ZEND_API void ZEND_FASTCALL _zend_hash_merge(HashTable *target, HashTable *source, copy_ctor_func_t pCopyConstructor, zend_bool overwrite ZEND_FILE_LINE_DC)
{
    uint32_t idx;
	Bucket *p;
	zval *t;

	IS_CONSISTENT(source);
	IS_CONSISTENT(target);
	HT_ASSERT(GC_REFCOUNT(target) == 1);

	if (overwrite) {
		for (idx = 0; idx < source->nNumUsed; idx++) {
			p = source->arData + idx;
			if (UNEXPECTED(Z_TYPE(p->val) == IS_UNDEF)) continue;
			if (UNEXPECTED(Z_TYPE(p->val) == IS_INDIRECT) &&
			    UNEXPECTED(Z_TYPE_P(Z_INDIRECT(p->val)) == IS_UNDEF)) {
			    continue;
			}
			if (p->key) {
				t = _zend_hash_add_or_update_i(target, p->key, &p->val, HASH_UPDATE | HASH_UPDATE_INDIRECT ZEND_FILE_LINE_RELAY_CC);
				if (t && pCopyConstructor) {
					pCopyConstructor(t);
				}
			} else {
				t = zend_hash_index_update(target, p->h, &p->val);
				if (t && pCopyConstructor) {
					pCopyConstructor(t);
				}
			}
		}
	} else {
		for (idx = 0; idx < source->nNumUsed; idx++) {
			p = source->arData + idx;
			if (UNEXPECTED(Z_TYPE(p->val) == IS_UNDEF)) continue;
			if (UNEXPECTED(Z_TYPE(p->val) == IS_INDIRECT) &&
			    UNEXPECTED(Z_TYPE_P(Z_INDIRECT(p->val)) == IS_UNDEF)) {
			    continue;
			}
			if (p->key) {
				t = _zend_hash_add_or_update_i(target, p->key, &p->val, HASH_ADD | HASH_UPDATE_INDIRECT ZEND_FILE_LINE_RELAY_CC);
				if (t && pCopyConstructor) {
					pCopyConstructor(t);
				}
			} else {
				t = zend_hash_index_add(target, p->h, &p->val);
				if (t && pCopyConstructor) {
					pCopyConstructor(t);
				}
			}
		}
	}
	if (target->nNumOfElements > 0) {
		idx = 0;
		while (Z_TYPE(target->arData[idx].val) == IS_UNDEF) {
			idx++;
		}
		target->nInternalPointer = idx;
	}
}
