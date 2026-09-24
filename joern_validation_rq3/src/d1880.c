static int init_mp_dev(struct pci_dev *pcidev, mppcibrd_t brd)
{
	static struct mp_device_t *sbdev = mp_devs;
	unsigned long addr = 0;
	int j;
	struct resource *ret = NULL;

	sbdev->device_id = brd.device_id;
	pci_read_config_byte(pcidev, PCI_CLASS_REVISION, &(sbdev->revision));
	sbdev->name = brd.name;
	sbdev->uart_access_addr = pcidev->resource[0].start & PCI_BASE_ADDRESS_IO_MASK;

	/* check revision. The SB16C1053APCI's option i/o address is BAR4 */
	if (sbdev->revision == 0xc0)
	{
		/* SB16C1053APCI */
		sbdev->option_reg_addr = pcidev->resource[4].start & PCI_BASE_ADDRESS_IO_MASK;
	}
	else
	{
		sbdev->option_reg_addr = pcidev->resource[1].start & PCI_BASE_ADDRESS_IO_MASK;
	}
#if 1	
	if (sbdev->revision == 0xc0)
	{
		outb(0x00, sbdev->option_reg_addr + MP_OPTR_GPOCR);
		inb(sbdev->option_reg_addr + MP_OPTR_GPOCR);
		outb(0x83, sbdev->option_reg_addr + MP_OPTR_GPOCR);
	}
#endif

	sbdev->irq = pcidev->irq;

	if ((brd.device_id & 0x0800) || !(brd.device_id &0xff00))
	{
		sbdev->poll_type = TYPE_INTERRUPT;
	}
	else
	{
		sbdev->poll_type = TYPE_POLL;
	}

	/* codes which is specific to each board*/
	switch(brd.device_id){
		case PCI_DEVICE_ID_MP1 :
		case PCIE_DEVICE_ID_MP1 :
		case PCIE_DEVICE_ID_MP1E :
		case PCIE_DEVICE_ID_GT_MP1 :
			sbdev->nr_ports = 1;
			break;
		case PCI_DEVICE_ID_MP2 :
		case PCIE_DEVICE_ID_MP2 :
		case PCIE_DEVICE_ID_GT_MP2 :
		case PCIE_DEVICE_ID_MP2B :
		case PCIE_DEVICE_ID_MP2E :
			sbdev->nr_ports = 2;

			/* serial base address remap */
			if (sbdev->revision == 0xc0)
			{
				int prev_port_addr = 0;

				pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_0, &prev_port_addr);
				pci_remap_base(pcidev, PCI_BASE_ADDRESS_1, prev_port_addr + 8, 8);
			}
			break;
		case PCI_DEVICE_ID_MP4 :
		case PCI_DEVICE_ID_MP4A :
		case PCIE_DEVICE_ID_MP4 :
		case PCI_DEVICE_ID_GT_MP4 :
		case PCI_DEVICE_ID_GT_MP4A :
		case PCIE_DEVICE_ID_GT_MP4 :
		case PCI_DEVICE_ID_MP4M :
		case PCIE_DEVICE_ID_MP4B :
			sbdev->nr_ports = 4;

			if(sbdev->revision == 0x91){
				sbdev->reserved_addr[0] = pcidev->resource[0].start & PCI_BASE_ADDRESS_IO_MASK;
				outb(0x03 , sbdev->reserved_addr[0] + 0x01);
				outb(0x03 , sbdev->reserved_addr[0] + 0x02);
				outb(0x01 , sbdev->reserved_addr[0] + 0x20);
				outb(0x00 , sbdev->reserved_addr[0] + 0x21);
				request_region(sbdev->reserved_addr[0], 32, sbdev->name);
				sbdev->uart_access_addr = pcidev->resource[1].start & PCI_BASE_ADDRESS_IO_MASK;
				sbdev->option_reg_addr = pcidev->resource[2].start & PCI_BASE_ADDRESS_IO_MASK;
			}

			/* SB16C1053APCI */
			if (sbdev->revision == 0xc0)
			{
				int prev_port_addr = 0;

				pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_0, &prev_port_addr);
				pci_remap_base(pcidev, PCI_BASE_ADDRESS_1, prev_port_addr + 8, 8);
				pci_remap_base(pcidev, PCI_BASE_ADDRESS_2, prev_port_addr + 16, 8);
				pci_remap_base(pcidev, PCI_BASE_ADDRESS_3, prev_port_addr + 24, 8);
			}
			break;
		case PCI_DEVICE_ID_MP6 :
		case PCI_DEVICE_ID_MP6A :
		case PCI_DEVICE_ID_GT_MP6 :
		case PCI_DEVICE_ID_GT_MP6A :
			sbdev->nr_ports = 6;

			/* SB16C1053APCI */
			if (sbdev->revision == 0xc0)
			{
				int prev_port_addr = 0;

				pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_0, &prev_port_addr);
				pci_remap_base(pcidev, PCI_BASE_ADDRESS_1, prev_port_addr + 8, 8);
				pci_remap_base(pcidev, PCI_BASE_ADDRESS_2, prev_port_addr + 16, 16);
				pci_remap_base(pcidev, PCI_BASE_ADDRESS_3, prev_port_addr + 32, 16);
			}
			break;
		case PCI_DEVICE_ID_MP8 :
		case PCIE_DEVICE_ID_MP8 :
		case PCI_DEVICE_ID_GT_MP8 :
		case PCIE_DEVICE_ID_GT_MP8 :
		case PCIE_DEVICE_ID_MP8B :
			sbdev->nr_ports = 8;
			break;
		case PCI_DEVICE_ID_MP32 :
		case PCIE_DEVICE_ID_MP32 :
		case PCI_DEVICE_ID_GT_MP32 :
		case PCIE_DEVICE_ID_GT_MP32 :
			{
				int portnum_hex=0;
				portnum_hex = inb(sbdev->option_reg_addr);
				sbdev->nr_ports = ((portnum_hex/16)*10) + (portnum_hex % 16);
			}
			break;
#ifdef CONFIG_PARPORT_PC
		case PCI_DEVICE_ID_MP2S1P :
			sbdev->nr_ports = 2;

			/* SB16C1053APCI */
			if (sbdev->revision == 0xc0)
			{
				int prev_port_addr = 0;

				pci_read_config_dword(pcidev, PCI_BASE_ADDRESS_0, &prev_port_addr);
				pci_remap_base(pcidev, PCI_BASE_ADDRESS_1, prev_port_addr + 8, 8);
			}

			/* add PC compatible parallel port */
			parport_pc_probe_port(pcidev->resource[2].start, pcidev->resource[3].start, PARPORT_IRQ_NONE, PARPORT_DMA_NONE, &pcidev->dev, 0);
			break;
		case PCI_DEVICE_ID_MP1P :
			/* add PC compatible parallel port */
			parport_pc_probe_port(pcidev->resource[2].start, pcidev->resource[3].start, PARPORT_IRQ_NONE, PARPORT_DMA_NONE, &pcidev->dev, 0);
			break;
#endif
	}

	ret = request_region(sbdev->uart_access_addr, (8*sbdev->nr_ports), sbdev->name);

	if (sbdev->revision == 0xc0)
	{
		ret = request_region(sbdev->option_reg_addr, 0x40, sbdev->name);
	}
	else
	{
		ret = request_region(sbdev->option_reg_addr, 0x20, sbdev->name);
	}


	NR_BOARD++;
	NR_PORTS += sbdev->nr_ports;

	/* Enable PCI interrupt */
	addr = sbdev->option_reg_addr + MP_OPTR_IMR0;
	for(j=0; j < (sbdev->nr_ports/8)+1; j++)
	{
		if (sbdev->poll_type == TYPE_INTERRUPT)
		{
			outb(0xff,addr +j);
		}
	}
	sbdev++;

	return 0;
}
