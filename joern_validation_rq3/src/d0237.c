X509_VERIFY_PARAM_set_purpose(X509_VERIFY_PARAM *param, int purpose)
{
	return X509_PURPOSE_set(&param->purpose, purpose);
}
