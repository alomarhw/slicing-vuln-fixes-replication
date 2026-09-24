static void ohci_async_cancel_device(OHCIState *ohci, USBDevice *dev)
{
    if (ohci->async_td &&
        usb_packet_is_inflight(&ohci->usb_packet) &&
        ohci->usb_packet.ep->dev == dev) {
        usb_cancel_packet(&ohci->usb_packet);
        ohci->async_td = 0;
    }
}
