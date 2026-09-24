OMX_ERRORTYPE omx_vdec::enable_adaptive_playback(unsigned long nMaxFrameWidth,
 unsigned long nMaxFrameHeight)
{

    OMX_ERRORTYPE eRet = OMX_ErrorNone;
 int ret = 0;
 unsigned long min_res_buf_count = 0;

    eRet = enable_smoothstreaming();
 if (eRet != OMX_ErrorNone) {
         DEBUG_PRINT_ERROR("Failed to enable Adaptive Playback on driver");
 return eRet;
 }

     DEBUG_PRINT_HIGH("Enabling Adaptive playback for %lu x %lu",
             nMaxFrameWidth,
             nMaxFrameHeight);
     m_smoothstreaming_mode = true;
     m_smoothstreaming_width = nMaxFrameWidth;
     m_smoothstreaming_height = nMaxFrameHeight;

 struct v4l2_format fmt;
     fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
     fmt.fmt.pix_mp.height = m_decoder_capability.min_height;
     fmt.fmt.pix_mp.width = m_decoder_capability.min_width;
     fmt.fmt.pix_mp.pixelformat = output_capability;

     ret = ioctl(drv_ctx.video_driver_fd, VIDIOC_S_FMT, &fmt);
 if (ret) {
         DEBUG_PRINT_ERROR("Set Resolution failed for HxW = %ux%u",
                           m_decoder_capability.min_height,
                           m_decoder_capability.min_width);
 return OMX_ErrorUnsupportedSetting;
 }

     eRet = get_buffer_req(&drv_ctx.op_buf);
 if (eRet != OMX_ErrorNone) {
         DEBUG_PRINT_ERROR("failed to get_buffer_req");
 return eRet;
 }

     min_res_buf_count = drv_ctx.op_buf.mincount;
     DEBUG_PRINT_LOW("enable adaptive - upper limit buffer count = %lu for HxW %ux%u",
                     min_res_buf_count, m_decoder_capability.min_height, m_decoder_capability.min_width);

     update_resolution(m_smoothstreaming_width, m_smoothstreaming_height,
                       m_smoothstreaming_width, m_smoothstreaming_height);
     eRet = is_video_session_supported();
 if (eRet != OMX_ErrorNone) {
         DEBUG_PRINT_ERROR("video session is not supported");
 return eRet;
 }

     fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
     fmt.fmt.pix_mp.height = drv_ctx.video_resolution.frame_height;
     fmt.fmt.pix_mp.width = drv_ctx.video_resolution.frame_width;
     fmt.fmt.pix_mp.pixelformat = output_capability;
     ret = ioctl(drv_ctx.video_driver_fd, VIDIOC_S_FMT, &fmt);
 if (ret) {
         DEBUG_PRINT_ERROR("Set Resolution failed for adaptive playback");
 return OMX_ErrorUnsupportedSetting;
 }

     eRet = get_buffer_req(&drv_ctx.op_buf);
 if (eRet != OMX_ErrorNone) {
         DEBUG_PRINT_ERROR("failed to get_buffer_req!!");
 return eRet;
 }
     DEBUG_PRINT_LOW("enable adaptive - upper limit buffer size = %u",
 (unsigned int)drv_ctx.op_buf.buffer_size);

     drv_ctx.op_buf.mincount = min_res_buf_count;
     drv_ctx.op_buf.actualcount = min_res_buf_count;
     eRet = set_buffer_req(&drv_ctx.op_buf);
 if (eRet != OMX_ErrorNone) {
         DEBUG_PRINT_ERROR("failed to set_buffer_req");
 return eRet;
 }

     eRet = get_buffer_req(&drv_ctx.op_buf);
 if (eRet != OMX_ErrorNone) {
         DEBUG_PRINT_ERROR("failed to get_buffer_req!!!");
 return eRet;
 }
     DEBUG_PRINT_HIGH("adaptive playback enabled, buf count = %u bufsize = %u",
                      drv_ctx.op_buf.mincount, (unsigned int)drv_ctx.op_buf.buffer_size);
 return eRet;
}
