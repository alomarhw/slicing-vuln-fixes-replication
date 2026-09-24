static long hiddev_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return hiddev_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
}
