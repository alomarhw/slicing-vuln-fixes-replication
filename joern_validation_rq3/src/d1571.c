int sas_notify_lldd_dev_found(struct domain_device *dev)
{
	int res = 0;
	struct sas_ha_struct *sas_ha = dev->port->ha;
	struct Scsi_Host *shost = sas_ha->core.shost;
	struct sas_internal *i = to_sas_internal(shost->transportt);

	if (!i->dft->lldd_dev_found)
		return 0;

	res = i->dft->lldd_dev_found(dev);
	if (res) {
		printk("sas: driver on pcidev %s cannot handle "
		       "device %llx, error:%d\n",
		       dev_name(sas_ha->dev),
		       SAS_ADDR(dev->sas_addr), res);
	}
	set_bit(SAS_DEV_FOUND, &dev->state);
	kref_get(&dev->kref);
	return res;
}
