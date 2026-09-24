static int setup_dev_symlinks(const struct lxc_rootfs *rootfs)
{
	char path[MAXPATHLEN];
	int ret,i;
	struct stat s;


	for (i = 0; i < sizeof(dev_symlinks) / sizeof(dev_symlinks[0]); i++) {
		const struct dev_symlinks *d = &dev_symlinks[i];
		ret = snprintf(path, sizeof(path), "%s/dev/%s", rootfs->path ? rootfs->mount : "", d->name);
		if (ret < 0 || ret >= MAXPATHLEN)
			return -1;

		/*
		 * Stat the path first.  If we don't get an error
		 * accept it as is and don't try to create it
		 */
		if (!stat(path, &s)) {
			continue;
		}

		ret = symlink(d->oldpath, path);

		if (ret && errno != EEXIST) {
			if ( errno == EROFS ) {
				WARN("Warning: Read Only file system while creating %s", path);
			} else {
				SYSERROR("Error creating %s", path);
				return -1;
			}
		}
	}
	return 0;
}
