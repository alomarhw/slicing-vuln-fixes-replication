static int attach_auth_trunc(struct xfrm_algo_auth **algpp, u8 *props,
			     struct nlattr *rta)
{
	struct xfrm_algo_auth *p, *ualg;
	struct xfrm_algo_desc *algo;

	if (!rta)
		return 0;

	ualg = nla_data(rta);

	algo = xfrm_aalg_get_byname(ualg->alg_name, 1);
	if (!algo)
		return -ENOSYS;
	if (ualg->alg_trunc_len > algo->uinfo.auth.icv_fullbits)
		return -EINVAL;
	*props = algo->desc.sadb_alg_id;

	p = kmemdup(ualg, xfrm_alg_auth_len(ualg), GFP_KERNEL);
	if (!p)
		return -ENOMEM;

	strcpy(p->alg_name, algo->name);
	if (!p->alg_trunc_len)
		p->alg_trunc_len = algo->uinfo.auth.icv_truncbits;

	*algpp = p;
	return 0;
}
