nfsd_foreach_client_openowner(struct nfs4_client *clp, u64 max,
			      struct list_head *collect,
			      void (*func)(struct nfs4_openowner *))
{
	struct nfs4_openowner *oop, *next;
	struct nfsd_net *nn = net_generic(current->nsproxy->net_ns,
						nfsd_net_id);
	u64 count = 0;

	lockdep_assert_held(&nn->client_lock);

	spin_lock(&clp->cl_lock);
	list_for_each_entry_safe(oop, next, &clp->cl_openowners, oo_perclient) {
		if (func) {
			func(oop);
			if (collect) {
				atomic_inc(&clp->cl_refcount);
				list_add(&oop->oo_perclient, collect);
			}
		}
		++count;
		/*
		 * Despite the fact that these functions deal with
		 * 64-bit integers for "count", we must ensure that
		 * it doesn't blow up the clp->cl_refcount. Throw a
		 * warning if we start to approach INT_MAX here.
		 */
		WARN_ON_ONCE(count == (INT_MAX / 2));
		if (count == max)
			break;
	}
	spin_unlock(&clp->cl_lock);

	return count;
}
