static noinline int btrfs_ioctl_ino_lookup(struct file *file,
					   void __user *argp)
{
	 struct btrfs_ioctl_ino_lookup_args *args;
	 struct inode *inode;
	 int ret;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	args = memdup_user(argp, sizeof(*args));
	if (IS_ERR(args))
		return PTR_ERR(args);

	inode = fdentry(file)->d_inode;

	if (args->treeid == 0)
		args->treeid = BTRFS_I(inode)->root->root_key.objectid;

	ret = btrfs_search_path_in_tree(BTRFS_I(inode)->root->fs_info,
					args->treeid, args->objectid,
					args->name);

	if (ret == 0 && copy_to_user(argp, args, sizeof(*args)))
		ret = -EFAULT;

	kfree(args);
	return ret;
}
