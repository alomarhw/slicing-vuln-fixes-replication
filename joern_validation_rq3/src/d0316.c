static int pcd_tray_move(struct cdrom_device_info *cdi, int position)
{
	char ej_cmd[12] = { 0x1b, 0, 0, 0, 3 - position, 0, 0, 0, 0, 0, 0, 0 };

	return pcd_atapi(cdi->handle, ej_cmd, 0, pcd_scratch,
			 position ? "eject" : "close tray");
}
