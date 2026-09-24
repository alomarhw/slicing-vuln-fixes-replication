ssize_t pdu_marshal(V9fsPDU *pdu, size_t offset, const char *fmt, ...)
{
    ssize_t ret;
    va_list ap;

    va_start(ap, fmt);
    ret = virtio_pdu_vmarshal(pdu, offset, fmt, ap);
    va_end(ap);

    return ret;
}
