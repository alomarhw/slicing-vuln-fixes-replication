alloc_perf_context(struct pmu *pmu, struct task_struct *task)
{
	struct perf_event_context *ctx;

	ctx = kzalloc(sizeof(struct perf_event_context), GFP_KERNEL);
	if (!ctx)
		return NULL;

	__perf_event_init_context(ctx);
	if (task) {
		ctx->task = task;
		get_task_struct(task);
	}
	ctx->pmu = pmu;

	return ctx;
}
