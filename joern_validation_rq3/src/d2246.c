static void compat_dh_get0_pqg(const DH *dh, const BIGNUM **p,
                               const BIGNUM **q, const BIGNUM **g)
{
    if (p != NULL)
        *p = dh->p;
    if (q != NULL)
        *q = dh->q;
    if (g != NULL)
        *g = dh->g;
}
