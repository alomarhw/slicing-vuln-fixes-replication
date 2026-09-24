static int apparmor_path_chmod(struct dentry *dentry, struct vfsmount *mnt,
			       mode_t mode)
{
	if (!mediated_filesystem(dentry->d_inode))
		return 0;

	return common_perm_mnt_dentry(OP_CHMOD, mnt, dentry, AA_MAY_CHMOD);
}
