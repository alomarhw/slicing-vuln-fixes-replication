static int composite_submit_control_transfer(int sub_api, struct usbi_transfer *itransfer)
{
	struct libusb_transfer *transfer = USBI_TRANSFER_TO_LIBUSB_TRANSFER(itransfer);
	struct libusb_context *ctx = DEVICE_CTX(transfer->dev_handle->dev);
	struct windows_device_priv *priv = _device_priv(transfer->dev_handle->dev);
	int i, pass;

	for (pass = 0; pass < 2; pass++) {
		for (i=0; i<USB_MAXINTERFACES; i++) {
			if (priv->usb_interface[i].path != NULL) {
				if ((pass == 0) && (priv->usb_interface[i].restricted_functionality)) {
					usbi_dbg("trying to skip restricted interface #%d (HID keyboard or mouse?)", i);
					continue;
				}
				usbi_dbg("using interface %d", i);
				return priv->usb_interface[i].apib->submit_control_transfer(priv->usb_interface[i].sub_api, itransfer);
			}
		}
	}

	usbi_err(ctx, "no libusbx supported interfaces to complete request");
	return LIBUSB_ERROR_NOT_FOUND;
}
