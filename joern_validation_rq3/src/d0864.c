vmxnet3_cleanup_msi(VMXNET3State *s)
{
    PCIDevice *d = PCI_DEVICE(s);

    if (s->msi_used) {
        msi_uninit(d);
    }
}
