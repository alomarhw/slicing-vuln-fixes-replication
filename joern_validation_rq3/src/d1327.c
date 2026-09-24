static void fm10k_init_reta(struct fm10k_intfc *interface)
{
	u16 i, rss_i = interface->ring_feature[RING_F_RSS].indices;
	u32 reta;

	/* If the Rx flow indirection table has been configured manually, we
	 * need to maintain it when possible.
	 */
	if (netif_is_rxfh_configured(interface->netdev)) {
		for (i = FM10K_RETA_SIZE; i--;) {
			reta = interface->reta[i];
			if ((((reta << 24) >> 24) < rss_i) &&
			    (((reta << 16) >> 24) < rss_i) &&
			    (((reta <<  8) >> 24) < rss_i) &&
			    (((reta)       >> 24) < rss_i))
				continue;

			/* this should never happen */
			dev_err(&interface->pdev->dev,
				"RSS indirection table assigned flows out of queue bounds. Reconfiguring.\n");
			goto repopulate_reta;
		}

		/* do nothing if all of the elements are in bounds */
		return;
	}

repopulate_reta:
	fm10k_write_reta(interface, NULL);
}
