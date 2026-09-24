ZEND_API int array_set_zval_key(HashTable *ht, zval *key, zval *value) /* {{{ */
{
	int result;

	switch (Z_TYPE_P(key)) {
		case IS_STRING:
			result = zend_symtable_update(ht, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, &value, sizeof(zval *), NULL);
			break;
		case IS_NULL:
			result = zend_symtable_update(ht, "", 1, &value, sizeof(zval *), NULL);
			break;
		case IS_RESOURCE:
			zend_error(E_STRICT, "Resource ID#%ld used as offset, casting to integer (%ld)", Z_LVAL_P(key), Z_LVAL_P(key));
			/* break missing intentionally */
		case IS_BOOL:
		case IS_LONG:
			result = zend_hash_index_update(ht, Z_LVAL_P(key), &value, sizeof(zval *), NULL);
			break;
		case IS_DOUBLE:
			result = zend_hash_index_update(ht, zend_dval_to_lval(Z_DVAL_P(key)), &value, sizeof(zval *), NULL);
			break;
		default:
			zend_error(E_WARNING, "Illegal offset type");
			result = FAILURE;
	}

	if (result == SUCCESS) {
		Z_ADDREF_P(value);
	}

	return result;
}
/* }}} */
