_set_batch_job_limits(slurm_msg_t *msg)
{
	int i;
	uint32_t alloc_lps = 0, last_bit = 0;
	bool cpu_log = slurm_get_debug_flags() & DEBUG_FLAG_CPU_BIND;
	slurm_cred_arg_t arg;
	batch_job_launch_msg_t *req = (batch_job_launch_msg_t *)msg->data;

	if (slurm_cred_get_args(req->cred, &arg) != SLURM_SUCCESS)
		return;
	req->job_core_spec = arg.job_core_spec;	/* Prevent user reset */

	if (cpu_log) {
		char *per_job = "";
		uint32_t job_mem  = arg.job_mem_limit;
		if (job_mem & MEM_PER_CPU) {
			job_mem &= (~MEM_PER_CPU);
			per_job = "_per_CPU";
		}
		info("====================");
		info("batch_job:%u job_mem:%uMB%s", req->job_id,
		     job_mem, per_job);
	}
	if (cpu_log || (arg.job_mem_limit & MEM_PER_CPU)) {
		if (arg.job_nhosts > 0) {
			last_bit = arg.sockets_per_node[0] *
				   arg.cores_per_socket[0];
			for (i=0; i<last_bit; i++) {
				if (!bit_test(arg.job_core_bitmap, i))
					continue;
				if (cpu_log)
					info("JobNode[0] CPU[%u] Job alloc",i);
				alloc_lps++;
			}
		}
		if (cpu_log)
			info("====================");
		if (alloc_lps == 0) {
			error("_set_batch_job_limit: alloc_lps is zero");
			alloc_lps = 1;
		}

		/* NOTE: alloc_lps is the count of allocated resources
		 * (typically cores). Convert to CPU count as needed */
		if (last_bit < 1)
			error("Batch job credential allocates no CPUs");
		else {
			i = conf->cpus / last_bit;
			if (i > 1)
				alloc_lps *= i;
		}
	}

	if (arg.job_mem_limit & MEM_PER_CPU) {
		req->job_mem = arg.job_mem_limit & (~MEM_PER_CPU);
		req->job_mem *= alloc_lps;
	} else
		req->job_mem = arg.job_mem_limit;

	slurm_cred_free_args(&arg);
}
