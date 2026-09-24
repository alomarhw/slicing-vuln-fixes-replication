static int find_cpu_to_read(struct perf_event *event, int local_cpu)
{
	int event_cpu = event->oncpu;
	u16 local_pkg, event_pkg;

	if (event->group_caps & PERF_EV_CAP_READ_ACTIVE_PKG) {
		event_pkg =  topology_physical_package_id(event_cpu);
		local_pkg =  topology_physical_package_id(local_cpu);

		if (event_pkg == local_pkg)
			return local_cpu;
	}

	return event_cpu;
}
