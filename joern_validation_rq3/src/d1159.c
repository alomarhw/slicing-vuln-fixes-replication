static int i8042_kbd_bind_notifier(struct notifier_block *nb,
				   unsigned long action, void *data)
{
	struct device *dev = data;
	struct serio *serio = to_serio_port(dev);
	struct i8042_port *port = serio->port_data;

	if (serio != i8042_ports[I8042_KBD_PORT_NO].serio)
		return 0;

	switch (action) {
	case BUS_NOTIFY_BOUND_DRIVER:
		port->driver_bound = true;
		break;

	case BUS_NOTIFY_UNBIND_DRIVER:
		port->driver_bound = false;
		break;
	}

	return 0;
}
