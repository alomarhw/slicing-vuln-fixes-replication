int ip_ra_control(struct sock *sk, unsigned char on,
		  void (*destructor)(struct sock *))
{
	struct ip_ra_chain *ra, *new_ra;
	struct ip_ra_chain __rcu **rap;

	if (sk->sk_type != SOCK_RAW || inet_sk(sk)->inet_num == IPPROTO_RAW)
		return -EINVAL;

	new_ra = on ? kmalloc(sizeof(*new_ra), GFP_KERNEL) : NULL;

	spin_lock_bh(&ip_ra_lock);
	for (rap = &ip_ra_chain;
	     (ra = rcu_dereference_protected(*rap,
			lockdep_is_held(&ip_ra_lock))) != NULL;
	     rap = &ra->next) {
		if (ra->sk == sk) {
			if (on) {
				spin_unlock_bh(&ip_ra_lock);
				kfree(new_ra);
				return -EADDRINUSE;
			}
			/* dont let ip_call_ra_chain() use sk again */
			ra->sk = NULL;
			rcu_assign_pointer(*rap, ra->next);
			spin_unlock_bh(&ip_ra_lock);

			if (ra->destructor)
				ra->destructor(sk);
			/*
			 * Delay sock_put(sk) and kfree(ra) after one rcu grace
			 * period. This guarantee ip_call_ra_chain() dont need
			 * to mess with socket refcounts.
			 */
			ra->saved_sk = sk;
			call_rcu(&ra->rcu, ip_ra_destroy_rcu);
			return 0;
		}
	}
	if (new_ra == NULL) {
		spin_unlock_bh(&ip_ra_lock);
		return -ENOBUFS;
	}
	new_ra->sk = sk;
	new_ra->destructor = destructor;

	new_ra->next = ra;
	rcu_assign_pointer(*rap, new_ra);
	sock_hold(sk);
	spin_unlock_bh(&ip_ra_lock);

	return 0;
}
