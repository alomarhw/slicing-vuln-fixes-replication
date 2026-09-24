static int __init atm_init(void)
{
	int error;

	error = proto_register(&vcc_proto, 0);
	if (error < 0)
		goto out;
	error = atmpvc_init();
	if (error < 0) {
		pr_err("atmpvc_init() failed with %d\n", error);
		goto out_unregister_vcc_proto;
	}
	error = atmsvc_init();
	if (error < 0) {
		pr_err("atmsvc_init() failed with %d\n", error);
		goto out_atmpvc_exit;
	}
	error = atm_proc_init();
	if (error < 0) {
		pr_err("atm_proc_init() failed with %d\n", error);
		goto out_atmsvc_exit;
	}
	error = atm_sysfs_init();
	if (error < 0) {
		pr_err("atm_sysfs_init() failed with %d\n", error);
		goto out_atmproc_exit;
	}
out:
	return error;
out_atmproc_exit:
	atm_proc_exit();
out_atmsvc_exit:
	atmsvc_exit();
out_atmpvc_exit:
	atmsvc_exit();
out_unregister_vcc_proto:
	proto_unregister(&vcc_proto);
	goto out;
}
