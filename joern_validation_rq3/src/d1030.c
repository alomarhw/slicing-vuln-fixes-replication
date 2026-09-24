bt_status_t btif_get_remote_device_property(bt_bdaddr_t *remote_addr,
 bt_property_type_t type)
{
 btif_storage_req_t req;

 if (!btif_is_enabled())
 return BT_STATUS_NOT_READY;

    memcpy(&(req.read_req.bd_addr), remote_addr, sizeof(bt_bdaddr_t));
    req.read_req.type = type;
 return btif_transfer_context(execute_storage_remote_request,
                                 BTIF_CORE_STORAGE_REMOTE_READ,
 (char*)&req, sizeof(btif_storage_req_t),
                                 NULL);
}
