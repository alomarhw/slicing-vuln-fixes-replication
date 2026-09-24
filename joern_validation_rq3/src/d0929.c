static int __init init_mqueue_fs(void)
{
	int error;

	mqueue_inode_cachep = kmem_cache_create("mqueue_inode_cache",
				sizeof(struct mqueue_inode_info), 0,
				SLAB_HWCACHE_ALIGN|SLAB_ACCOUNT, init_once);
	if (mqueue_inode_cachep == NULL)
		return -ENOMEM;

	/* ignore failures - they are not fatal */
	mq_sysctl_table = mq_register_sysctl_table();

	error = register_filesystem(&mqueue_fs_type);
	if (error)
		goto out_sysctl;

	spin_lock_init(&mq_lock);

	error = mq_init_ns(&init_ipc_ns);
	if (error)
		goto out_filesystem;

	return 0;

out_filesystem:
	unregister_filesystem(&mqueue_fs_type);
out_sysctl:
	if (mq_sysctl_table)
		unregister_sysctl_table(mq_sysctl_table);
	kmem_cache_destroy(mqueue_inode_cachep);
	return error;
}
