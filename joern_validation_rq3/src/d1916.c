rdpsnd_init(char *optarg)
{
	struct audio_driver *pos;
	char *driver = NULL, *options = NULL;

	drivers = NULL;

	packet.data = (uint8 *) xmalloc(65536);
	packet.p = packet.end = packet.data;
	packet.size = 0;

	rdpsnd_channel =
		channel_register("rdpsnd", CHANNEL_OPTION_INITIALIZED | CHANNEL_OPTION_ENCRYPT_RDP,
				 rdpsnd_process);

	rdpsnddbg_channel =
		channel_register("snddbg", CHANNEL_OPTION_INITIALIZED | CHANNEL_OPTION_ENCRYPT_RDP,
				 rdpsnddbg_process);

	if ((rdpsnd_channel == NULL) || (rdpsnddbg_channel == NULL))
	{
		logger(Sound, Error,
		       "rdpsnd_init(), failed to register rdpsnd / snddbg virtual channels");
		return False;
	}

	rdpsnd_queue_init();

	if (optarg != NULL && strlen(optarg) > 0)
	{
		driver = options = optarg;

		while (*options != '\0' && *options != ':')
			options++;

		if (*options == ':')
		{
			*options = '\0';
			options++;
		}

		if (*options == '\0')
			options = NULL;
	}

	rdpsnd_register_drivers(options);

	if (!driver)
		return True;

	pos = drivers;
	while (pos != NULL)
	{
		if (!strcmp(pos->name, driver))
		{
			logger(Sound, Debug, "rdpsnd_init(), using driver '%s'", pos->name);
			current_driver = pos;
			return True;
		}
		pos = pos->next;
	}
	return False;
}
