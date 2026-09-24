static void vmxnet3_put_int_state(QEMUFile *f, void *pv, size_t size)
{
    Vmxnet3IntState *r = pv;

    qemu_put_byte(f, r->is_masked);
    qemu_put_byte(f, r->is_pending);
    qemu_put_byte(f, r->is_asserted);
}
