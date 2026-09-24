static int __dev_close(struct net_device *dev)
{
	int retval;
	LIST_HEAD(single);

	list_add(&dev->close_list, &single);
	retval = __dev_close_many(&single);
	list_del(&single);

	return retval;
}
