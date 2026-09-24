static void pmcraid_get_fwversion_done(struct pmcraid_cmd *cmd)
{
	struct pmcraid_instance *pinstance = cmd->drv_inst;
	u32 ioasc = le32_to_cpu(cmd->ioa_cb->ioasa.ioasc);
	unsigned long lock_flags;

	/* configuration table entry size depends on firmware version. If fw
	 * version is not known, it is not possible to interpret IOA config
	 * table
	 */
	if (ioasc) {
		pmcraid_err("IOA Inquiry failed with %x\n", ioasc);
		spin_lock_irqsave(pinstance->host->host_lock, lock_flags);
		pinstance->ioa_state = IOA_STATE_IN_RESET_ALERT;
		pmcraid_reset_alert(cmd);
		spin_unlock_irqrestore(pinstance->host->host_lock, lock_flags);
	} else  {
		pmcraid_querycfg(cmd);
	}
}
