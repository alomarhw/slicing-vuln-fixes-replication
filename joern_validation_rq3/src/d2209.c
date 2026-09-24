void __cleanup_signal(struct signal_struct *sig)
{
	thread_group_cputime_free(sig);
	tty_kref_put(sig->tty);
	kmem_cache_free(signal_cachep, sig);
}
