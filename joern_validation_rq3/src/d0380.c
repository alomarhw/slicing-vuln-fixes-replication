static void pvscsi_realize(DeviceState *qdev, Error **errp)
{
    PVSCSIClass *pvs_c = PVSCSI_DEVICE_GET_CLASS(qdev);
    PCIDevice *pci_dev = PCI_DEVICE(qdev);
    PVSCSIState *s = PVSCSI(qdev);

    if (!(s->compat_flags & PVSCSI_COMPAT_DISABLE_PCIE)) {
        pci_dev->cap_present |= QEMU_PCI_CAP_EXPRESS;
    }

    pvs_c->parent_dc_realize(qdev, errp);
}
