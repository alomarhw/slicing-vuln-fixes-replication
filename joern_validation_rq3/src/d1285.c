void vfio_pci_intx_unmask(struct vfio_pci_device *vdev)
{
	if (vfio_pci_intx_unmask_handler(vdev, NULL) > 0)
		vfio_send_intx_eventfd(vdev, NULL);
}
