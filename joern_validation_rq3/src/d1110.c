WORD32 ih264d_insert_st_node(dpb_manager_t *ps_dpb_mgr,
 struct pic_buffer_t *ps_pic_buf,
                          UWORD8 u1_buf_id,
                          UWORD32 u4_cur_pic_num)
{
    WORD32 i;
 struct dpb_info_t *ps_dpb_info = ps_dpb_mgr->as_dpb_info;
    UWORD8 u1_picture_type = ps_pic_buf->u1_picturetype;
 /* Find an unused dpb location */
 for(i = 0; i < MAX_REF_BUFS; i++)
 {
 if((ps_dpb_info[i].ps_pic_buf == ps_pic_buf)
 && ps_dpb_info[i].u1_used_as_ref)
 {
 /* Can occur only for field bottom pictures */
            ps_dpb_info[i].s_bot_field.u1_reference_info = IS_SHORT_TERM;

 /*signal an error in the case of frame pic*/
 if(ps_dpb_info[i].ps_pic_buf->u1_pic_type == FRM_PIC)
 {
 return ERROR_DBP_MANAGER_T;
 }
 else
 {
 return OK;
 }
 }

 if((ps_dpb_info[i].u1_used_as_ref == UNUSED_FOR_REF)
 && (ps_dpb_info[i].s_top_field.u1_reference_info
 == UNUSED_FOR_REF)
 && (ps_dpb_info[i].s_bot_field.u1_reference_info
 == UNUSED_FOR_REF))
 break;
 }
 if(i == MAX_REF_BUFS)
 {
        UWORD32 i4_error_code;
        i4_error_code = ERROR_DBP_MANAGER_T;
 return i4_error_code;
 }

 /* Create dpb info */
    ps_dpb_info[i].ps_pic_buf = ps_pic_buf;
    ps_dpb_info[i].ps_prev_short = ps_dpb_mgr->ps_dpb_st_head;
    ps_dpb_info[i].u1_buf_id = u1_buf_id;
    ps_dpb_info[i].u1_used_as_ref = TRUE;
    ps_dpb_info[i].u1_lt_idx = MAX_REF_BUFS + 1;
    ps_dpb_info[i].i4_frame_num = u4_cur_pic_num;
    ps_dpb_info[i].ps_pic_buf->i4_frame_num = u4_cur_pic_num;

 /* update the head node of linked list to point to the cur Pic */
    ps_dpb_mgr->ps_dpb_st_head = ps_dpb_info + i;

    ps_dpb_mgr->u1_num_st_ref_bufs++;
 /* Identify the picture as a short term picture buffer */
    ps_pic_buf->u1_is_short = IS_SHORT_TERM;

 if((u1_picture_type & 0x03) == FRM_PIC)
 {
        ps_dpb_info[i].u1_used_as_ref = IS_SHORT_TERM;
        ps_dpb_info[i].s_top_field.u1_reference_info = IS_SHORT_TERM;
        ps_dpb_info[i].s_bot_field.u1_reference_info = IS_SHORT_TERM;
 }

 if((u1_picture_type & 0x03) == TOP_FLD)
        ps_dpb_info[i].s_top_field.u1_reference_info = IS_SHORT_TERM;

 if((u1_picture_type & 0x03) == BOT_FLD)
        ps_dpb_info[i].s_bot_field.u1_reference_info = IS_SHORT_TERM;

 return OK;
}
