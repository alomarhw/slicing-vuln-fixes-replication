static void vmxnet3_realize(DeviceState *qdev, Error **errp)
{
    VMXNET3Class *vc = VMXNET3_DEVICE_GET_CLASS(qdev);
    PCIDevice *pci_dev = PCI_DEVICE(qdev);
    VMXNET3State *s = VMXNET3(qdev);

    if (!(s->compat_flags & VMXNET3_COMPAT_FLAG_DISABLE_PCIE)) {
        pci_dev->cap_present |= QEMU_PCI_CAP_EXPRESS;
    }

    vc->parent_dc_realize(qdev, errp);
}
