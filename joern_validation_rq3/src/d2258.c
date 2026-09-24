ftrace_trace_snapshot_callback(struct trace_array *tr, struct ftrace_hash *hash,
			       char *glob, char *cmd, char *param, int enable)
{
	struct ftrace_probe_ops *ops;
	void *count = (void *)-1;
	char *number;
	int ret;

	if (!tr)
		return -ENODEV;

	/* hash funcs only work with set_ftrace_filter */
	if (!enable)
		return -EINVAL;

	ops = param ? &snapshot_count_probe_ops :  &snapshot_probe_ops;

	if (glob[0] == '!')
		return unregister_ftrace_function_probe_func(glob+1, tr, ops);

	if (!param)
		goto out_reg;

	number = strsep(&param, ":");

	if (!strlen(number))
		goto out_reg;

	/*
	 * We use the callback data field (which is a pointer)
	 * as our counter.
	 */
	ret = kstrtoul(number, 0, (unsigned long *)&count);
	if (ret)
		return ret;

 out_reg:
	ret = tracing_alloc_snapshot_instance(tr);
	if (ret < 0)
		goto out;

	ret = register_ftrace_function_probe(glob, tr, ops, count);

 out:
	return ret < 0 ? ret : 0;
}
