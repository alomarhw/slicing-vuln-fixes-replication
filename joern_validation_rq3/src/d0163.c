static union _zend_function *dbstmt_method_get(zval **object_pp, char *method_name, int method_len, const zend_literal *key TSRMLS_DC)
{
	zend_function *fbc = NULL;
	char *lc_method_name;
	zval *object = *object_pp;

	lc_method_name = emalloc(method_len + 1);
	zend_str_tolower_copy(lc_method_name, method_name, method_len);

	if (zend_hash_find(&Z_OBJCE_P(object)->function_table, lc_method_name,
			method_len+1, (void**)&fbc) == FAILURE) {

		pdo_stmt_t *stmt = (pdo_stmt_t*)zend_object_store_get_object(object TSRMLS_CC);
		/* instance not created by PDO object */
		if (!stmt->dbh) {
			goto out;
		}
		/* not a pre-defined method, nor a user-defined method; check
		 * the driver specific methods */
		if (!stmt->dbh->cls_methods[PDO_DBH_DRIVER_METHOD_KIND_STMT]) {
			if (!pdo_hash_methods(stmt->dbh,
				PDO_DBH_DRIVER_METHOD_KIND_STMT TSRMLS_CC)
				|| !stmt->dbh->cls_methods[PDO_DBH_DRIVER_METHOD_KIND_STMT]) {
				goto out;
			}
		}

		if (zend_hash_find(stmt->dbh->cls_methods[PDO_DBH_DRIVER_METHOD_KIND_STMT],
				lc_method_name, method_len+1, (void**)&fbc) == FAILURE) {
			goto out;
		}
		/* got it */
	}

out:
	if (!fbc) {
		fbc = std_object_handlers.get_method(object_pp, method_name, method_len, key TSRMLS_CC);
	}
	efree(lc_method_name);
	return fbc;
}
