bool btif_get_address_type(const BD_ADDR bd_addr, int *p_addr_type)
{
 if (p_addr_type == NULL)
 return FALSE;

 bt_bdaddr_t bda;
    bdcpy(bda.address, bd_addr);

 bdstr_t bd_addr_str;
    bdaddr_to_string(&bda, bd_addr_str, sizeof(bd_addr_str));

 if (!btif_config_get_int(bd_addr_str, "AddrType", p_addr_type))
 return FALSE;

    LOG_DEBUG("%s: Device [%s] address type %d", __FUNCTION__, bd_addr_str, *p_addr_type);
 return TRUE;
}
