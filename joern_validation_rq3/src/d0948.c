static int __init cma_init(void)
{
	int ret;

	cma_wq = create_singlethread_workqueue("rdma_cm");
	if (!cma_wq)
		return -ENOMEM;

	ib_sa_register_client(&sa_client);
	rdma_addr_register_client(&addr_client);
	register_netdevice_notifier(&cma_nb);

	ret = ib_register_client(&cma_client);
	if (ret)
		goto err;

	if (ibnl_add_client(RDMA_NL_RDMA_CM, RDMA_NL_RDMA_CM_NUM_OPS, cma_cb_table))
		printk(KERN_WARNING "RDMA CMA: failed to add netlink callback\n");

	return 0;

err:
	unregister_netdevice_notifier(&cma_nb);
	rdma_addr_unregister_client(&addr_client);
	ib_sa_unregister_client(&sa_client);
	destroy_workqueue(cma_wq);
	return ret;
}
