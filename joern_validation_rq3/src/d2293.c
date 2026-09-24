PHP_PGSQL_API int php_pgsql_insert(PGconn *pg_link, const char *table, zval *var_array, zend_ulong opt, zend_string **sql)
{
	zval *val, converted;
	char buf[256];
	char *tmp;
	smart_str querystr = {0};
	int ret = FAILURE;
	zend_ulong num_idx;
	zend_string *fld;

	assert(pg_link != NULL);
	assert(table != NULL);
	assert(Z_TYPE_P(var_array) == IS_ARRAY);

	ZVAL_UNDEF(&converted);
	if (zend_hash_num_elements(Z_ARRVAL_P(var_array)) == 0) {
		smart_str_appends(&querystr, "INSERT INTO ");
		build_tablename(&querystr, pg_link, table);
		smart_str_appends(&querystr, " DEFAULT VALUES");

		goto no_values;
	}

	/* convert input array if needed */
	if (!(opt & (PGSQL_DML_NO_CONV|PGSQL_DML_ESCAPE))) {
		array_init(&converted);
		if (php_pgsql_convert(pg_link, table, var_array, &converted, (opt & PGSQL_CONV_OPTS)) == FAILURE) {
			goto cleanup;
		}
		var_array = &converted;
	}

	smart_str_appends(&querystr, "INSERT INTO ");
	build_tablename(&querystr, pg_link, table);
	smart_str_appends(&querystr, " (");

	ZEND_HASH_FOREACH_KEY(Z_ARRVAL_P(var_array), num_idx, fld) {
		if (fld == NULL) {
			php_error_docref(NULL, E_NOTICE, "Expects associative array for values to be inserted");
			goto cleanup;
		}
		if (opt & PGSQL_DML_ESCAPE) {
			tmp = PGSQLescapeIdentifier(pg_link, fld->val, fld->len + 1);
			smart_str_appends(&querystr, tmp);
			PGSQLfree(tmp);
		} else {
			smart_str_appendl(&querystr, fld->val, fld->len);
		}
		smart_str_appendc(&querystr, ',');
	} ZEND_HASH_FOREACH_END();
	querystr.s->len--;
	smart_str_appends(&querystr, ") VALUES (");
	
	/* make values string */
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(var_array), val) {
		/* we can avoid the key_type check here, because we tested it in the other loop */
		switch (Z_TYPE_P(val)) {
			case IS_STRING:
				if (opt & PGSQL_DML_ESCAPE) {
					size_t new_len;
					char *tmp;
					tmp = (char *)safe_emalloc(Z_STRLEN_P(val), 2, 1);
					new_len = PQescapeStringConn(pg_link, tmp, Z_STRVAL_P(val), Z_STRLEN_P(val), NULL);
					smart_str_appendc(&querystr, '\'');
					smart_str_appendl(&querystr, tmp, new_len);
					smart_str_appendc(&querystr, '\'');
					efree(tmp);
				} else {
					smart_str_appendl(&querystr, Z_STRVAL_P(val), Z_STRLEN_P(val));
				}
				break;
			case IS_LONG:
				smart_str_append_long(&querystr, Z_LVAL_P(val));
				break;
			case IS_DOUBLE:
				smart_str_appendl(&querystr, buf, snprintf(buf, sizeof(buf), "%F", Z_DVAL_P(val)));
				break;
			case IS_NULL:
				smart_str_appendl(&querystr, "NULL", sizeof("NULL")-1);
				break;
			default:
				php_error_docref(NULL, E_WARNING, "Expects scaler values. type = %d", Z_TYPE_P(val));
				goto cleanup;
				break;
		}
		smart_str_appendc(&querystr, ',');
	} ZEND_HASH_FOREACH_END();
	/* Remove the trailing "," */
	querystr.s->len--;
	smart_str_appends(&querystr, ");");

no_values:

	smart_str_0(&querystr);

	if ((opt & (PGSQL_DML_EXEC|PGSQL_DML_ASYNC)) &&
		do_exec(&querystr, PGRES_COMMAND_OK, pg_link, (opt & PGSQL_CONV_OPTS)) == 0) {
		ret = SUCCESS;
	}
	else if (opt & PGSQL_DML_STRING) {
		ret = SUCCESS;
	}
	
cleanup:
	zval_ptr_dtor(&converted);
	if (ret == SUCCESS && (opt & PGSQL_DML_STRING)) {
		*sql = querystr.s;
	}
	else {
		smart_str_free(&querystr);
	}
	return ret;
}
