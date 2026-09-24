static zend_object_value date_object_clone_timezone(zval *this_ptr TSRMLS_DC)
{
	php_timezone_obj *new_obj = NULL;
	php_timezone_obj *old_obj = (php_timezone_obj *) zend_object_store_get_object(this_ptr TSRMLS_CC);
	zend_object_value new_ov = date_object_new_timezone_ex(old_obj->std.ce, &new_obj TSRMLS_CC);
	
	zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);
	if (!old_obj->initialized) {
		return new_ov;
	}
	
	new_obj->type = old_obj->type;
	new_obj->initialized = 1;
	switch (new_obj->type) {
		case TIMELIB_ZONETYPE_ID:
			new_obj->tzi.tz = old_obj->tzi.tz;
			break;
		case TIMELIB_ZONETYPE_OFFSET:
			new_obj->tzi.utc_offset = old_obj->tzi.utc_offset;
			break;
		case TIMELIB_ZONETYPE_ABBR:
			new_obj->tzi.z.utc_offset = old_obj->tzi.z.utc_offset;
			new_obj->tzi.z.dst        = old_obj->tzi.z.dst;
			new_obj->tzi.z.abbr       = strdup(old_obj->tzi.z.abbr);
			break;
	}
	
	return new_ov;
}
