static void efx_flush_all(struct efx_nic *efx)
{
	/* Make sure the hardware monitor is stopped */
	cancel_delayed_work_sync(&efx->monitor_work);
	/* Stop scheduled port reconfigurations */
	cancel_work_sync(&efx->mac_work);
}
