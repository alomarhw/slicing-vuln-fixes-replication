static void digi_release(struct usb_serial *serial)
{
	struct digi_serial *serial_priv;
	struct digi_port *priv;

	serial_priv = usb_get_serial_data(serial);

	priv = usb_get_serial_port_data(serial_priv->ds_oob_port);
	kfree(priv);

	kfree(serial_priv);
}
