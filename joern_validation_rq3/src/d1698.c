make_s2kparams(int value, size_t len, krb5_data **ps2kparams)
{
    krb5_data *s2kparams;
    krb5_error_code ret;

    ALLOC(s2kparams);
    if (s2kparams == NULL)
	return ENOMEM;
    ret = krb5_data_alloc(s2kparams, len);
    if (ret) {
	free(s2kparams);
	return ret;
    }
    _krb5_put_int(s2kparams->data, value, len);
    *ps2kparams = s2kparams;
    return 0;
}
