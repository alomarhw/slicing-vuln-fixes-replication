PHP_FUNCTION(openssl_private_decrypt)
{
	zval *key, *crypted;
	EVP_PKEY *pkey;
	int cryptedlen;
	zend_string *cryptedbuf = NULL;
	unsigned char *crypttemp;
	int successful = 0;
	zend_long padding = RSA_PKCS1_PADDING;
	zend_resource *keyresource = NULL;
	char * data;
	size_t data_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sz/z|l", &data, &data_len, &crypted, &key, &padding) == FAILURE) {
		return;
	}
	RETVAL_FALSE;

	pkey = php_openssl_evp_from_zval(key, 0, "", 0, 0, &keyresource);
	if (pkey == NULL) {
		php_error_docref(NULL, E_WARNING, "key parameter is not a valid private key");
		RETURN_FALSE;
	}

	PHP_OPENSSL_CHECK_SIZE_T_TO_INT(data_len, data);

	cryptedlen = EVP_PKEY_size(pkey);
	crypttemp = emalloc(cryptedlen + 1);

	switch (EVP_PKEY_id(pkey)) {
		case EVP_PKEY_RSA:
		case EVP_PKEY_RSA2:
			cryptedlen = RSA_private_decrypt((int)data_len,
					(unsigned char *)data,
					crypttemp,
					EVP_PKEY_get0_RSA(pkey),
					(int)padding);
			if (cryptedlen != -1) {
				cryptedbuf = zend_string_alloc(cryptedlen, 0);
				memcpy(ZSTR_VAL(cryptedbuf), crypttemp, cryptedlen);
				successful = 1;
			}
			break;
		default:
			php_error_docref(NULL, E_WARNING, "key type not supported in this PHP build!");
	}

	efree(crypttemp);

	if (successful) {
		zval_dtor(crypted);
		ZSTR_VAL(cryptedbuf)[cryptedlen] = '\0';
		ZVAL_NEW_STR(crypted, cryptedbuf);
		cryptedbuf = NULL;
		RETVAL_TRUE;
	}

	if (keyresource == NULL) {
		EVP_PKEY_free(pkey);
	}
	if (cryptedbuf) {
		zend_string_release(cryptedbuf);
	}
}
