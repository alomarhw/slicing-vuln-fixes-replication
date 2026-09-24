int sr_tray_move(struct cdrom_device_info *cdi, int pos)
{
	Scsi_CD *cd = cdi->handle;
	struct packet_command cgc;

	memset(&cgc, 0, sizeof(struct packet_command));
	cgc.cmd[0] = GPCMD_START_STOP_UNIT;
	cgc.cmd[4] = (pos == 0) ? 0x03 /* close */ : 0x02 /* eject */ ;
	cgc.data_direction = DMA_NONE;
	cgc.timeout = IOCTL_TIMEOUT;
	return sr_do_ioctl(cd, &cgc);
}
