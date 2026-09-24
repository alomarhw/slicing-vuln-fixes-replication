struct inode *ovl_d_select_inode(struct dentry *dentry, unsigned file_flags)
{
	int err;
	struct path realpath;
	enum ovl_path_type type;

	if (d_is_dir(dentry))
		return d_backing_inode(dentry);

	type = ovl_path_real(dentry, &realpath);
	if (ovl_open_need_copy_up(file_flags, type, realpath.dentry)) {
		err = ovl_want_write(dentry);
		if (err)
			return ERR_PTR(err);

		if (file_flags & O_TRUNC)
			err = ovl_copy_up_last(dentry, NULL, true);
		else
			err = ovl_copy_up(dentry);
		ovl_drop_write(dentry);
		if (err)
			return ERR_PTR(err);

		ovl_path_upper(dentry, &realpath);
	}

	if (realpath.dentry->d_flags & DCACHE_OP_SELECT_INODE)
		return realpath.dentry->d_op->d_select_inode(realpath.dentry, file_flags);

	return d_backing_inode(realpath.dentry);
}
