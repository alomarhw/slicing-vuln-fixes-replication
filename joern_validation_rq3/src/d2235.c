static int get_bootfile_path(const char *file_path, char *bootfile_path,
			     size_t bootfile_path_size)
{
	char *bootfile, *last_slash;
	size_t path_len = 0;

	/* Only syslinux allows absolute paths */
	if (file_path[0] == '/' && !is_pxe)
		goto ret;

	bootfile = from_env("bootfile");

	if (!bootfile)
		goto ret;

	last_slash = strrchr(bootfile, '/');

	if (!last_slash)
		goto ret;

	path_len = (last_slash - bootfile) + 1;

	if (bootfile_path_size < path_len) {
		printf("bootfile_path too small. (%zd < %zd)\n",
		       bootfile_path_size, path_len);

		return -1;
	}

	strncpy(bootfile_path, bootfile, path_len);

 ret:
	bootfile_path[path_len] = '\0';

	return 1;
}
