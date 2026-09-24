static int aac_probe_one(struct pci_dev *pdev, const struct pci_device_id *id)
{
	unsigned index = id->driver_data;
	struct Scsi_Host *shost;
	struct aac_dev *aac;
	struct list_head *insert = &aac_devices;
	int error = -ENODEV;
	int unique_id = 0;
	u64 dmamask;
	extern int aac_sync_mode;

	list_for_each_entry(aac, &aac_devices, entry) {
		if (aac->id > unique_id)
			break;
		insert = &aac->entry;
		unique_id++;
	}

	pci_disable_link_state(pdev, PCIE_LINK_STATE_L0S | PCIE_LINK_STATE_L1 |
			       PCIE_LINK_STATE_CLKPM);

	error = pci_enable_device(pdev);
	if (error)
		goto out;
	error = -ENODEV;

	/*
	 * If the quirk31 bit is set, the adapter needs adapter
	 * to driver communication memory to be allocated below 2gig
	 */
	if (aac_drivers[index].quirks & AAC_QUIRK_31BIT)
		dmamask = DMA_BIT_MASK(31);
	else
		dmamask = DMA_BIT_MASK(32);

	if (pci_set_dma_mask(pdev, dmamask) ||
			pci_set_consistent_dma_mask(pdev, dmamask))
		goto out_disable_pdev;

	pci_set_master(pdev);

	shost = scsi_host_alloc(&aac_driver_template, sizeof(struct aac_dev));
	if (!shost)
		goto out_disable_pdev;

	shost->irq = pdev->irq;
	shost->unique_id = unique_id;
	shost->max_cmd_len = 16;

	aac = (struct aac_dev *)shost->hostdata;
	aac->base_start = pci_resource_start(pdev, 0);
	aac->scsi_host_ptr = shost;
	aac->pdev = pdev;
	aac->name = aac_driver_template.name;
	aac->id = shost->unique_id;
	aac->cardtype = index;
	INIT_LIST_HEAD(&aac->entry);

	aac->fibs = kzalloc(sizeof(struct fib) * (shost->can_queue + AAC_NUM_MGT_FIB), GFP_KERNEL);
	if (!aac->fibs)
		goto out_free_host;
	spin_lock_init(&aac->fib_lock);

	/*
	 *	Map in the registers from the adapter.
	 */
	aac->base_size = AAC_MIN_FOOTPRINT_SIZE;
	if ((*aac_drivers[index].init)(aac))
		goto out_unmap;

	if (aac->sync_mode) {
		if (aac_sync_mode)
			printk(KERN_INFO "%s%d: Sync. mode enforced "
				"by driver parameter. This will cause "
				"a significant performance decrease!\n",
				aac->name,
				aac->id);
		else
			printk(KERN_INFO "%s%d: Async. mode not supported "
				"by current driver, sync. mode enforced."
				"\nPlease update driver to get full performance.\n",
				aac->name,
				aac->id);
	}

	/*
	 *	Start any kernel threads needed
	 */
	aac->thread = kthread_run(aac_command_thread, aac, AAC_DRIVERNAME);
	if (IS_ERR(aac->thread)) {
		printk(KERN_ERR "aacraid: Unable to create command thread.\n");
		error = PTR_ERR(aac->thread);
		aac->thread = NULL;
		goto out_deinit;
	}

	/*
	 * If we had set a smaller DMA mask earlier, set it to 4gig
	 * now since the adapter can dma data to at least a 4gig
	 * address space.
	 */
	if (aac_drivers[index].quirks & AAC_QUIRK_31BIT)
		if (pci_set_dma_mask(pdev, DMA_BIT_MASK(32)))
			goto out_deinit;

	aac->maximum_num_channels = aac_drivers[index].channels;
	error = aac_get_adapter_info(aac);
	if (error < 0)
		goto out_deinit;

	/*
	 * Lets override negotiations and drop the maximum SG limit to 34
	 */
	if ((aac_drivers[index].quirks & AAC_QUIRK_34SG) &&
			(shost->sg_tablesize > 34)) {
		shost->sg_tablesize = 34;
		shost->max_sectors = (shost->sg_tablesize * 8) + 112;
	}

	if ((aac_drivers[index].quirks & AAC_QUIRK_17SG) &&
			(shost->sg_tablesize > 17)) {
		shost->sg_tablesize = 17;
		shost->max_sectors = (shost->sg_tablesize * 8) + 112;
	}

	error = pci_set_dma_max_seg_size(pdev,
		(aac->adapter_info.options & AAC_OPT_NEW_COMM) ?
			(shost->max_sectors << 9) : 65536);
	if (error)
		goto out_deinit;

	/*
	 * Firmware printf works only with older firmware.
	 */
	if (aac_drivers[index].quirks & AAC_QUIRK_34SG)
		aac->printf_enabled = 1;
	else
		aac->printf_enabled = 0;

	/*
	 * max channel will be the physical channels plus 1 virtual channel
	 * all containers are on the virtual channel 0 (CONTAINER_CHANNEL)
	 * physical channels are address by their actual physical number+1
	 */
	if (aac->nondasd_support || expose_physicals || aac->jbod)
		shost->max_channel = aac->maximum_num_channels;
	else
		shost->max_channel = 0;

	aac_get_config_status(aac, 0);
	aac_get_containers(aac);
	list_add(&aac->entry, insert);

	shost->max_id = aac->maximum_num_containers;
	if (shost->max_id < aac->maximum_num_physicals)
		shost->max_id = aac->maximum_num_physicals;
	if (shost->max_id < MAXIMUM_NUM_CONTAINERS)
		shost->max_id = MAXIMUM_NUM_CONTAINERS;
	else
		shost->this_id = shost->max_id;

	/*
	 * dmb - we may need to move the setting of these parms somewhere else once
	 * we get a fib that can report the actual numbers
	 */
	shost->max_lun = AAC_MAX_LUN;

	pci_set_drvdata(pdev, shost);

	error = scsi_add_host(shost, &pdev->dev);
	if (error)
		goto out_deinit;
	scsi_scan_host(shost);

	return 0;

 out_deinit:
	__aac_shutdown(aac);
 out_unmap:
	aac_fib_map_free(aac);
	if (aac->comm_addr)
		pci_free_consistent(aac->pdev, aac->comm_size, aac->comm_addr,
		  aac->comm_phys);
	kfree(aac->queues);
	aac_adapter_ioremap(aac, 0);
	kfree(aac->fibs);
	kfree(aac->fsa_dev);
 out_free_host:
	scsi_host_put(shost);
 out_disable_pdev:
	pci_disable_device(pdev);
 out:
	return error;
}
