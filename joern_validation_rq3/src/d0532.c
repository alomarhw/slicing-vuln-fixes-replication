static int dev_match_devt(struct device *dev, const void *data)
{
	const dev_t *devt = data;
	return dev->devt == *devt;
}
