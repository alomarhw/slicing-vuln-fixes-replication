static int au1100fb_setup(struct au1100fb_device *fbdev)
{
	char *this_opt, *options;
	int num_panels = ARRAY_SIZE(known_lcd_panels);

	if (num_panels <= 0) {
		print_err("No LCD panels supported by driver!");
		return -ENODEV;
	}

	if (fb_get_options(DRIVER_NAME, &options))
		return -ENODEV;
	if (!options)
		return -ENODEV;

	while ((this_opt = strsep(&options, ",")) != NULL) {
		/* Panel option */
		if (!strncmp(this_opt, "panel:", 6)) {
			int i;
			this_opt += 6;
			for (i = 0; i < num_panels; i++) {
				if (!strncmp(this_opt, known_lcd_panels[i].name,
					     strlen(this_opt))) {
					fbdev->panel = &known_lcd_panels[i];
					fbdev->panel_idx = i;
					break;
				}
			}
			if (i >= num_panels) {
				print_warn("Panel '%s' not supported!", this_opt);
				return -ENODEV;
			}
		}
		/* Unsupported option */
		else
			print_warn("Unsupported option \"%s\"", this_opt);
	}

	print_info("Panel=%s", fbdev->panel->name);

	return 0;
}
