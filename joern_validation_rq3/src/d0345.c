xdr_krb5_key_salt_tuple(XDR *xdrs, krb5_key_salt_tuple *objp)
{
    if (!xdr_krb5_enctype(xdrs, &objp->ks_enctype))
	return FALSE;
    if (!xdr_krb5_salttype(xdrs, &objp->ks_salttype))
	return FALSE;
    return TRUE;
}
