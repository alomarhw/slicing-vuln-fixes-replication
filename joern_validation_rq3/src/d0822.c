BIGNUM *BN_new(void)
	{
	BIGNUM *ret;

	if ((ret=(BIGNUM *)OPENSSL_malloc(sizeof(BIGNUM))) == NULL)
		{
		BNerr(BN_F_BN_NEW,ERR_R_MALLOC_FAILURE);
		return(NULL);
		}
	ret->flags=BN_FLG_MALLOCED;
	ret->top=0;
	ret->neg=0;
	ret->dmax=0;
	ret->d=NULL;
	bn_check_top(ret);
	return(ret);
	}
