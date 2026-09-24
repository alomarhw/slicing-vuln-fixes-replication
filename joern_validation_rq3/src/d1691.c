PHP_FUNCTION(openssl_csr_get_public_key)
{
	zval * zcsr;
	zend_bool use_shortnames = 1;
	zend_resource *csr_resource;

	X509_REQ * csr;
	EVP_PKEY *tpubkey;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|b", &zcsr, &use_shortnames) == FAILURE) {
		return;
	}

	csr = php_openssl_csr_from_zval(zcsr, 0, &csr_resource);

	if (csr == NULL) {
		RETURN_FALSE;
	}

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
	/* Due to changes in OpenSSL 1.1 related to locking when decoding CSR,
	 * the pub key is not changed after assigning. It means if we pass
	 * a private key, it will be returned including the private part.
	 * If we duplicate it, then we get just the public part which is
	 * the same behavior as for OpenSSL 1.0 */
	csr = X509_REQ_dup(csr);
#endif
	/* Retrieve the public key from the CSR */
	tpubkey = X509_REQ_get_pubkey(csr);

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
	/* We need to free the CSR as it was duplicated */
	X509_REQ_free(csr);
#endif

	if (tpubkey == NULL) {
		php_openssl_store_errors();
		RETURN_FALSE;
	}

	RETURN_RES(zend_register_resource(tpubkey, le_key));
}
