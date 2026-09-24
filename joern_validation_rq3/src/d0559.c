static ssize_t pmcraid_show_log_level(
	struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct Scsi_Host *shost = class_to_shost(dev);
	struct pmcraid_instance *pinstance =
		(struct pmcraid_instance *)shost->hostdata;
	return snprintf(buf, PAGE_SIZE, "%d\n", pinstance->current_log_level);
}
