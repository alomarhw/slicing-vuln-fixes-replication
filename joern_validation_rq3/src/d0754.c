find_KRB5SignedPath(krb5_context context,
		    const AuthorizationData *ad,
		    krb5_data *data)
{
    AuthorizationData child;
    krb5_error_code ret;
    int pos;

    if (ad == NULL || ad->len == 0)
	return KRB5KDC_ERR_PADATA_TYPE_NOSUPP;

    pos = ad->len - 1;

    if (ad->val[pos].ad_type != KRB5_AUTHDATA_IF_RELEVANT)
	return KRB5KDC_ERR_PADATA_TYPE_NOSUPP;

    ret = decode_AuthorizationData(ad->val[pos].ad_data.data,
				   ad->val[pos].ad_data.length,
				   &child,
				   NULL);
    if (ret) {
	krb5_set_error_message(context, ret, "Failed to decode "
			       "IF_RELEVANT with %d", ret);
	return ret;
    }

    if (child.len != 1) {
	free_AuthorizationData(&child);
	return KRB5KDC_ERR_PADATA_TYPE_NOSUPP;
    }

    if (child.val[0].ad_type != KRB5_AUTHDATA_SIGNTICKET) {
	free_AuthorizationData(&child);
	return KRB5KDC_ERR_PADATA_TYPE_NOSUPP;
    }

    if (data)
	ret = der_copy_octet_string(&child.val[0].ad_data, data);
    free_AuthorizationData(&child);
    return ret;
}
