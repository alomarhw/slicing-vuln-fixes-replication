static void rds_inc_addref(struct rds_incoming *inc)
{
	rdsdebug("addref inc %p ref %d\n", inc, refcount_read(&inc->i_refcount));
	refcount_inc(&inc->i_refcount);
}
