IV_API_CALL_STATUS_T ih264d_api_function(iv_obj_t *dec_hdl,
 void *pv_api_ip,
 void *pv_api_op)
{
    UWORD32 command;
    UWORD32 *pu2_ptr_cmd;
    UWORD32 u4_api_ret;
    IV_API_CALL_STATUS_T e_status;
    e_status = api_check_struct_sanity(dec_hdl, pv_api_ip, pv_api_op);

 if(e_status != IV_SUCCESS)
 {
        UWORD32 *ptr_err;

        ptr_err = (UWORD32 *)pv_api_op;
        UNUSED(ptr_err);
        H264_DEC_DEBUG_PRINT("error code = %d\n", *(ptr_err + 1));
 return IV_FAIL;
 }

    pu2_ptr_cmd = (UWORD32 *)pv_api_ip;
    pu2_ptr_cmd++;

    command = *pu2_ptr_cmd;
 switch(command)
 {

 case IVD_CMD_CREATE:
            u4_api_ret = ih264d_create(dec_hdl, (void *)pv_api_ip,
 (void *)pv_api_op);
 break;
 case IVD_CMD_DELETE:
            u4_api_ret = ih264d_delete(dec_hdl, (void *)pv_api_ip,
 (void *)pv_api_op);
 break;

 case IVD_CMD_VIDEO_DECODE:
            u4_api_ret = ih264d_video_decode(dec_hdl, (void *)pv_api_ip,
 (void *)pv_api_op);
 break;

 case IVD_CMD_GET_DISPLAY_FRAME:
            u4_api_ret = ih264d_get_display_frame(dec_hdl, (void *)pv_api_ip,
 (void *)pv_api_op);

 break;

 case IVD_CMD_SET_DISPLAY_FRAME:
            u4_api_ret = ih264d_set_display_frame(dec_hdl, (void *)pv_api_ip,
 (void *)pv_api_op);

 break;

 case IVD_CMD_REL_DISPLAY_FRAME:
            u4_api_ret = ih264d_rel_display_frame(dec_hdl, (void *)pv_api_ip,
 (void *)pv_api_op);
 break;

 case IVD_CMD_VIDEO_CTL:
            u4_api_ret = ih264d_ctl(dec_hdl, (void *)pv_api_ip,
 (void *)pv_api_op);
 break;
 default:
            u4_api_ret = IV_FAIL;
 break;
 }

 return u4_api_ret;
}
