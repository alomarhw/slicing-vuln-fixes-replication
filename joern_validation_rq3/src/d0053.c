EC_KEY *d2i_EC_PUBKEY_fp(FILE *fp, EC_KEY **eckey)
	{
	return ASN1_d2i_fp_of(EC_KEY,EC_KEY_new,d2i_EC_PUBKEY,fp,eckey);
	}
