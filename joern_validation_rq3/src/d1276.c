int apply_subsystem_event_filter(struct trace_subsystem_dir *dir,
				 char *filter_string)
{
	struct event_subsystem *system = dir->subsystem;
	struct trace_array *tr = dir->tr;
	struct event_filter *filter = NULL;
	int err = 0;

	mutex_lock(&event_mutex);

	/* Make sure the system still has events */
	if (!dir->nr_events) {
		err = -ENODEV;
		goto out_unlock;
	}

	if (!strcmp(strstrip(filter_string), "0")) {
		filter_free_subsystem_preds(dir, tr);
		remove_filter_string(system->filter);
		filter = system->filter;
		system->filter = NULL;
		/* Ensure all filters are no longer used */
		synchronize_sched();
		filter_free_subsystem_filters(dir, tr);
		__free_filter(filter);
		goto out_unlock;
	}

	err = create_system_filter(dir, tr, filter_string, &filter);
	if (filter) {
		/*
		 * No event actually uses the system filter
		 * we can free it without synchronize_sched().
		 */
		__free_filter(system->filter);
		system->filter = filter;
	}
out_unlock:
	mutex_unlock(&event_mutex);

	return err;
}
