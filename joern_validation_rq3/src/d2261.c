static void _make_prolog_mem_container(slurm_msg_t *msg)
{
	prolog_launch_msg_t *req = (prolog_launch_msg_t *)msg->data;
	job_mem_limits_t *job_limits_ptr;
	step_loc_t step_info;

	_convert_job_mem(msg);	/* Convert per-CPU mem limit */
	if (req->job_mem_limit) {
		slurm_mutex_lock(&job_limits_mutex);
		if (!job_limits_list)
			job_limits_list = list_create(_job_limits_free);
		step_info.jobid  = req->job_id;
		step_info.stepid = SLURM_EXTERN_CONT;
		job_limits_ptr = list_find_first (job_limits_list,
						  _step_limits_match,
						  &step_info);
		if (!job_limits_ptr) {
			job_limits_ptr = xmalloc(sizeof(job_mem_limits_t));
			job_limits_ptr->job_id   = req->job_id;
			job_limits_ptr->job_mem  = req->job_mem_limit;
			job_limits_ptr->step_id  = SLURM_EXTERN_CONT;
			job_limits_ptr->step_mem = req->job_mem_limit;
#if _LIMIT_INFO
			info("AddLim step:%u.%u job_mem:%u step_mem:%u",
			      job_limits_ptr->job_id, job_limits_ptr->step_id,
			      job_limits_ptr->job_mem,
			      job_limits_ptr->step_mem);
#endif
			list_append(job_limits_list, job_limits_ptr);
		}
		slurm_mutex_unlock(&job_limits_mutex);
	}
}
