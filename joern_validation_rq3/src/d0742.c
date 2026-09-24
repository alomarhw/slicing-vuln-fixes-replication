ext4_set_acl(struct inode *inode, struct posix_acl *acl, int type)
{
	handle_t *handle;
	int error, retries = 0;

retry:
	handle = ext4_journal_start(inode, EXT4_HT_XATTR,
				    ext4_jbd2_credits_xattr(inode));
	if (IS_ERR(handle))
		return PTR_ERR(handle);

	error = __ext4_set_acl(handle, inode, type, acl);
	ext4_journal_stop(handle);
	if (error == -ENOSPC && ext4_should_retry_alloc(inode->i_sb, &retries))
		goto retry;
	return error;
}
