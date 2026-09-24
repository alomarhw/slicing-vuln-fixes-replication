fm_mgr_config_connect
(
	IN      p_fm_config_conx_hdlt       p_hdl
)
{
	fm_config_conx_hdl      *hdl = p_hdl;
	fm_mgr_config_errno_t   res = FM_CONF_OK;
	int						fail_count = 0;

    if ( (res = fm_mgr_config_mgr_connect(hdl, FM_MGR_SM)) == FM_CONF_INIT_ERR )
    {
        res = FM_CONF_INIT_ERR;
        goto cleanup;
    }

    if(res != FM_CONF_OK)
        fail_count++;

    if ( (res = fm_mgr_config_mgr_connect(hdl, FM_MGR_PM)) == FM_CONF_INIT_ERR )
    {
        res = FM_CONF_INIT_ERR;
        goto cleanup;
    }

    if(res != FM_CONF_OK)
        fail_count++;

    if ( (res = fm_mgr_config_mgr_connect(hdl, FM_MGR_FE)) == FM_CONF_INIT_ERR )
    {
        res = FM_CONF_INIT_ERR;
        goto cleanup;
    }

    if(res != FM_CONF_OK)
        fail_count++;

    if(fail_count < 4){
        res = FM_CONF_OK;
    }

	cleanup:

	return res;
}
