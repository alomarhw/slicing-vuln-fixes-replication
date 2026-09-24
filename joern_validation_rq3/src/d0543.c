static int shmem_create(struct inode *dir, struct dentry *dentry, umode_t mode,
		bool excl)
{
	return shmem_mknod(dir, dentry, mode | S_IFREG, 0);
}
