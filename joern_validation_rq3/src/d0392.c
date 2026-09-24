static long macvtap_compat_ioctl(struct file *file, unsigned int cmd,
				 unsigned long arg)
{
	return macvtap_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
}
