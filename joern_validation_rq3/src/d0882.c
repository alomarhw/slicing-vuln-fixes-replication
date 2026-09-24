static const char *ovl_follow_link(struct dentry *dentry, void **cookie)
{
	struct dentry *realdentry;
	struct inode *realinode;
	struct ovl_link_data *data = NULL;
	const char *ret;

	realdentry = ovl_dentry_real(dentry);
	realinode = realdentry->d_inode;

	if (WARN_ON(!realinode->i_op->follow_link))
		return ERR_PTR(-EPERM);

	if (realinode->i_op->put_link) {
		data = kmalloc(sizeof(struct ovl_link_data), GFP_KERNEL);
		if (!data)
			return ERR_PTR(-ENOMEM);
		data->realdentry = realdentry;
	}

	ret = realinode->i_op->follow_link(realdentry, cookie);
	if (IS_ERR_OR_NULL(ret)) {
		kfree(data);
		return ret;
	}

	if (data)
		data->cookie = *cookie;

	*cookie = data;

	return ret;
}
