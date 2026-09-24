void cache_tmp_xattr(struct file_struct *file, stat_x *sxp)
{
	int ndx;

	if (!sxp->xattr)
		return;

	if (prior_xattr_count == (size_t)-1)
		prior_xattr_count = rsync_xal_l.count;
	ndx = find_matching_xattr(sxp->xattr);
	if (ndx < 0)
		rsync_xal_store(sxp->xattr); /* adds item to rsync_xal_l */

	F_XATTR(file) = ndx;
}
