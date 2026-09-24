xfs_vn_mknod(
	struct inode	*dir,
	struct dentry	*dentry,
	umode_t		mode,
	dev_t		rdev)
{
	return xfs_generic_create(dir, dentry, mode, rdev, false);
}
