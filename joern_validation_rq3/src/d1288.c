void perf_cgroup_switch(struct task_struct *task, int mode)
{
	struct perf_cpu_context *cpuctx;
	struct pmu *pmu;
	unsigned long flags;

	/*
	 * disable interrupts to avoid geting nr_cgroup
	 * changes via __perf_event_disable(). Also
	 * avoids preemption.
	 */
	local_irq_save(flags);

	/*
	 * we reschedule only in the presence of cgroup
	 * constrained events.
	 */
	rcu_read_lock();

	list_for_each_entry_rcu(pmu, &pmus, entry) {
		cpuctx = this_cpu_ptr(pmu->pmu_cpu_context);

		/*
		 * perf_cgroup_events says at least one
		 * context on this CPU has cgroup events.
		 *
		 * ctx->nr_cgroups reports the number of cgroup
		 * events for a context.
		 */
		if (cpuctx->ctx.nr_cgroups > 0) {
			perf_ctx_lock(cpuctx, cpuctx->task_ctx);
			perf_pmu_disable(cpuctx->ctx.pmu);

			if (mode & PERF_CGROUP_SWOUT) {
				cpu_ctx_sched_out(cpuctx, EVENT_ALL);
				/*
				 * must not be done before ctxswout due
				 * to event_filter_match() in event_sched_out()
				 */
				cpuctx->cgrp = NULL;
			}

			if (mode & PERF_CGROUP_SWIN) {
				WARN_ON_ONCE(cpuctx->cgrp);
				/* set cgrp before ctxsw in to
				 * allow event_filter_match() to not
				 * have to pass task around
				 */
				cpuctx->cgrp = perf_cgroup_from_task(task);
				cpu_ctx_sched_in(cpuctx, EVENT_ALL, task);
			}
			perf_pmu_enable(cpuctx->ctx.pmu);
			perf_ctx_unlock(cpuctx, cpuctx->task_ctx);
		}
	}

	rcu_read_unlock();

	local_irq_restore(flags);
}
