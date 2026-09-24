void tracing_record_taskinfo(struct task_struct *task, int flags)
{
	bool done;

	if (tracing_record_taskinfo_skip(flags))
		return;

	/*
	 * Record as much task information as possible. If some fail, continue
	 * to try to record the others.
	 */
	done = !(flags & TRACE_RECORD_CMDLINE) || trace_save_cmdline(task);
	done &= !(flags & TRACE_RECORD_TGID) || trace_save_tgid(task);

	/* If recording any information failed, retry again soon. */
	if (!done)
		return;

	__this_cpu_write(trace_taskinfo_save, false);
}
