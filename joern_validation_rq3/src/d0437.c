struct kobject *dm_kobject(struct mapped_device *md)
{
	return &md->kobj_holder.kobj;
}
