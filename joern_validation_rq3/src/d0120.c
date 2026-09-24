static ssize_t lbs_bcnmiss_write(struct file *file, const char __user *userbuf,
				 size_t count, loff_t *ppos)
{
	return lbs_threshold_write(TLV_TYPE_BCNMISS, CMD_SUBSCRIBE_BCNMISS,
				   file, userbuf, count, ppos);
}
