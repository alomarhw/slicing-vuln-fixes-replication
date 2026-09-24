static void opl3_reset(int devno)
{
	int i;

	for (i = 0; i < 18; i++)
		devc->lv_map[i] = i;

	for (i = 0; i < devc->nr_voice; i++)
	{
		opl3_command(pv_map[devc->lv_map[i]].ioaddr,
			KSL_LEVEL + pv_map[devc->lv_map[i]].op[0], 0xff);

		opl3_command(pv_map[devc->lv_map[i]].ioaddr,
			KSL_LEVEL + pv_map[devc->lv_map[i]].op[1], 0xff);

		if (pv_map[devc->lv_map[i]].voice_mode == 4)
		{
			opl3_command(pv_map[devc->lv_map[i]].ioaddr,
				KSL_LEVEL + pv_map[devc->lv_map[i]].op[2], 0xff);

			opl3_command(pv_map[devc->lv_map[i]].ioaddr,
				KSL_LEVEL + pv_map[devc->lv_map[i]].op[3], 0xff);
		}

		opl3_kill_note(devno, i, 0, 64);
	}

	if (devc->model == 2)
	{
		devc->v_alloc->max_voice = devc->nr_voice = 18;

		for (i = 0; i < 18; i++)
			pv_map[i].voice_mode = 2;

	}
}
