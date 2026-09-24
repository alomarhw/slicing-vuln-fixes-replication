static void put_event(struct perf_event *event)
{
	struct perf_event_context *ctx = event->ctx;

	if (!atomic_long_dec_and_test(&event->refcount))
		return;

	if (!is_kernel_event(event))
		perf_remove_from_owner(event);

	WARN_ON_ONCE(ctx->parent_ctx);
	/*
	 * There are two ways this annotation is useful:
	 *
	 *  1) there is a lock recursion from perf_event_exit_task
	 *     see the comment there.
	 *
	 *  2) there is a lock-inversion with mmap_sem through
	 *     perf_event_read_group(), which takes faults while
	 *     holding ctx->mutex, however this is called after
	 *     the last filedesc died, so there is no possibility
	 *     to trigger the AB-BA case.
	 */
	mutex_lock_nested(&ctx->mutex, SINGLE_DEPTH_NESTING);
	perf_remove_from_context(event, true);
	mutex_unlock(&ctx->mutex);

	_free_event(event);
}
