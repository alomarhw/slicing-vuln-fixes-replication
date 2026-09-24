static int fuse_fsync(struct file *file, int datasync)
{
	return fuse_fsync_common(file, datasync, 0);
}
