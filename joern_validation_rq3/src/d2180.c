static int __init ext4_init_feat_adverts(void)
{
	struct ext4_features *ef;
	int ret = -ENOMEM;

	ef = kzalloc(sizeof(struct ext4_features), GFP_KERNEL);
	if (!ef)
		goto out;

	ef->f_kobj.kset = ext4_kset;
	init_completion(&ef->f_kobj_unregister);
	ret = kobject_init_and_add(&ef->f_kobj, &ext4_feat_ktype, NULL,
				   "features");
	if (ret) {
		kfree(ef);
		goto out;
	}

	ext4_feat = ef;
	ret = 0;
out:
	return ret;
}
