static inline void perf_cgroup_sched_in(struct task_struct *prev,
					struct task_struct *task)
{
	struct perf_cgroup *cgrp1;
	struct perf_cgroup *cgrp2 = NULL;

	/*
	 * we come here when we know perf_cgroup_events > 0
	 */
	cgrp1 = perf_cgroup_from_task(task);

	/* prev can never be NULL */
	cgrp2 = perf_cgroup_from_task(prev);

	/*
	 * only need to schedule in cgroup events if we are changing
	 * cgroup during ctxsw. Cgroup events were not scheduled
	 * out of ctxsw out if that was not the case.
	 */
	if (cgrp1 != cgrp2)
		perf_cgroup_switch(task, PERF_CGROUP_SWIN);
}
