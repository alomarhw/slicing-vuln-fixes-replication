SPL_METHOD(SplObjectStorage, removeAllExcept)
{
	zval *obj;
	spl_SplObjectStorage *intern = (spl_SplObjectStorage *)zend_object_store_get_object(getThis() TSRMLS_CC);
	spl_SplObjectStorage *other;
	spl_SplObjectStorageElement *element;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &obj, spl_ce_SplObjectStorage) == FAILURE) {
		return;
	}

	other = (spl_SplObjectStorage *)zend_object_store_get_object(obj TSRMLS_CC);

	zend_hash_internal_pointer_reset(&intern->storage);
	while (zend_hash_get_current_data(&intern->storage, (void **)&element) == SUCCESS) {
		if (!spl_object_storage_contains(other, getThis(), element->obj TSRMLS_CC)) {
			spl_object_storage_detach(intern, getThis(), element->obj TSRMLS_CC);
		}
		zend_hash_move_forward(&intern->storage);
	}

	zend_hash_internal_pointer_reset_ex(&intern->storage, &intern->pos);
	intern->index = 0;

	RETURN_LONG(zend_hash_num_elements(&intern->storage));
}
