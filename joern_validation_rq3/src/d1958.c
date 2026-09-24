void btif_dm_save_ble_bonding_keys(void)
{

 bt_bdaddr_t bd_addr;

    BTIF_TRACE_DEBUG("%s",__FUNCTION__ );

    bdcpy(bd_addr.address, pairing_cb.bd_addr);

 if (pairing_cb.ble.is_penc_key_rcvd)
 {
        btif_storage_add_ble_bonding_key(&bd_addr,
 (char *) &pairing_cb.ble.penc_key,
                                         BTIF_DM_LE_KEY_PENC,
 sizeof(tBTM_LE_PENC_KEYS));
 }

 if (pairing_cb.ble.is_pid_key_rcvd)
 {
        btif_storage_add_ble_bonding_key(&bd_addr,
 (char *) &pairing_cb.ble.pid_key,
                                         BTIF_DM_LE_KEY_PID,
 sizeof(tBTM_LE_PID_KEYS));
 }


 if (pairing_cb.ble.is_pcsrk_key_rcvd)
 {
        btif_storage_add_ble_bonding_key(&bd_addr,
 (char *) &pairing_cb.ble.pcsrk_key,
                                         BTIF_DM_LE_KEY_PCSRK,
 sizeof(tBTM_LE_PCSRK_KEYS));
 }


 if (pairing_cb.ble.is_lenc_key_rcvd)
 {
        btif_storage_add_ble_bonding_key(&bd_addr,
 (char *) &pairing_cb.ble.lenc_key,
                                         BTIF_DM_LE_KEY_LENC,
 sizeof(tBTM_LE_LENC_KEYS));
 }

 if (pairing_cb.ble.is_lcsrk_key_rcvd)
 {
        btif_storage_add_ble_bonding_key(&bd_addr,
 (char *) &pairing_cb.ble.lcsrk_key,
                                         BTIF_DM_LE_KEY_LCSRK,
 sizeof(tBTM_LE_LCSRK_KEYS));
 }

 if (pairing_cb.ble.is_lidk_key_rcvd)
 {
        btif_storage_add_ble_bonding_key(&bd_addr,
                                         NULL,
                                         BTIF_DM_LE_KEY_LID,
 0);
 }

}
