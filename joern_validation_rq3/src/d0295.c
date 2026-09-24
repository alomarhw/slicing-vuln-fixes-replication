static int cgfs_nrtasks(void *hdata)
{
	struct cgfs_data *d = hdata;
	struct cgroup_process_info *info;
	struct cgroup_mount_point *mp = NULL;
	char *abs_path = NULL;
	int ret;

	if (!d) {
		errno = ENOENT;
		return -1;
	}

	info = d->info;
	if (!info) {
		errno = ENOENT;
		return -1;
	}

	if (info->designated_mount_point) {
		mp = info->designated_mount_point;
	} else {
		mp = lxc_cgroup_find_mount_point(info->hierarchy, info->cgroup_path, false);
		if (!mp)
			return -1;
	}

	abs_path = cgroup_to_absolute_path(mp, info->cgroup_path, NULL);
	if (!abs_path)
		return -1;

	ret = cgroup_recursive_task_count(abs_path);
	free(abs_path);
	return ret;
}
