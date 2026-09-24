void sas_smp_handler(struct bsg_job *job, struct Scsi_Host *shost,
		struct sas_rphy *rphy)
{
	struct domain_device *dev;
	unsigned int reslen = 0;
	int ret = -EINVAL;

	/* no rphy means no smp target support (ie aic94xx host) */
	if (!rphy)
		return sas_smp_host_handler(job, shost);

	switch (rphy->identify.device_type) {
	case SAS_EDGE_EXPANDER_DEVICE:
	case SAS_FANOUT_EXPANDER_DEVICE:
		break;
	default:
		printk("%s: can we send a smp request to a device?\n",
		       __func__);
		goto out;
	}

	dev = sas_find_dev_by_rphy(rphy);
	if (!dev) {
		printk("%s: fail to find a domain_device?\n", __func__);
		goto out;
	}

	/* do we need to support multiple segments? */
	if (job->request_payload.sg_cnt > 1 ||
	    job->reply_payload.sg_cnt > 1) {
		printk("%s: multiple segments req %u, rsp %u\n",
		       __func__, job->request_payload.payload_len,
		       job->reply_payload.payload_len);
		goto out;
	}

	ret = smp_execute_task_sg(dev, job->request_payload.sg_list,
			job->reply_payload.sg_list);
	if (ret > 0) {
		/* positive number is the untransferred residual */
		reslen = ret;
		ret = 0;
	}

out:
	bsg_job_done(job, ret, reslen);
}
