sd_show_allow_restart(struct device *dev, struct device_attribute *attr,
		      char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);

	return snprintf(buf, 40, "%d\n", sdkp->device->allow_restart);
}
