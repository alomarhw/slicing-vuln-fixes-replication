static int dm_wait_for_completion(struct mapped_device *md, long task_state)
{
	int r = 0;
	DEFINE_WAIT(wait);

	while (1) {
		prepare_to_wait(&md->wait, &wait, task_state);

		if (!md_in_flight(md))
			break;

		if (signal_pending_state(task_state, current)) {
			r = -EINTR;
			break;
		}

		io_schedule();
	}
	finish_wait(&md->wait, &wait);

	return r;
}
