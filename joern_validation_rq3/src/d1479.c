static inline bool cgfs_create_legacy(void *hdata, pid_t pid)
{
	struct cgfs_data *d = hdata;
	struct cgroup_process_info *i;

	if (!d)
		return false;
	i = d->info;
	if (lxc_cgroup_create_legacy(i, d->name, pid) < 0) {
		ERROR("failed to create legacy ns cgroups for '%s'", d->name);
		return false;
	}
	return true;
}
