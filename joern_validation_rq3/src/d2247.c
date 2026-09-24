static int force_hcd_device_descriptor(struct libusb_device *dev)
{
	struct windows_device_priv *parent_priv, *priv = _device_priv(dev);
	struct libusb_context *ctx = DEVICE_CTX(dev);
	int vid, pid;

	dev->num_configurations = 1;
	priv->dev_descriptor.bLength = sizeof(USB_DEVICE_DESCRIPTOR);
	priv->dev_descriptor.bDescriptorType = USB_DEVICE_DESCRIPTOR_TYPE;
	priv->dev_descriptor.bNumConfigurations = 1;
	priv->active_config = 1;

	if (priv->parent_dev == NULL) {
		usbi_err(ctx, "program assertion failed - HCD hub has no parent");
		return LIBUSB_ERROR_NO_DEVICE;
	}
	parent_priv = _device_priv(priv->parent_dev);
	if (sscanf(parent_priv->path, "\\\\.\\PCI#VEN_%04x&DEV_%04x%*s", &vid, &pid) == 2) {
		priv->dev_descriptor.idVendor = (uint16_t)vid;
		priv->dev_descriptor.idProduct = (uint16_t)pid;
	} else {
		usbi_warn(ctx, "could not infer VID/PID of HCD hub from '%s'", parent_priv->path);
		priv->dev_descriptor.idVendor = 0x1d6b;		// Linux Foundation root hub
		priv->dev_descriptor.idProduct = 1;
	}
	return LIBUSB_SUCCESS;
}
