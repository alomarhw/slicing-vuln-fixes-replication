static ssize_t show_cons_active(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct console *cs[16];
	int i = 0;
	struct console *c;
	ssize_t count = 0;

	console_lock();
	for_each_console(c) {
		if (!c->device)
			continue;
		if (!c->write)
			continue;
		if ((c->flags & CON_ENABLED) == 0)
			continue;
		cs[i++] = c;
		if (i >= ARRAY_SIZE(cs))
			break;
	}
	while (i--) {
		int index = cs[i]->index;
		struct tty_driver *drv = cs[i]->device(cs[i], &index);

		/* don't resolve tty0 as some programs depend on it */
		if (drv && (cs[i]->index > 0 || drv->major != TTY_MAJOR))
			count += tty_line_name(drv, index, buf + count);
		else
			count += sprintf(buf + count, "%s%d",
					 cs[i]->name, cs[i]->index);

		count += sprintf(buf + count, "%c", i ? ' ':'\n');
	}
	console_unlock();

	return count;
}
