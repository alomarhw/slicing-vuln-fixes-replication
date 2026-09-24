int release_vp9_frame_buffer(void *user_priv,
 vpx_codec_frame_buffer_t *fb) {
 ExternalFrameBufferList *const fb_list =
 reinterpret_cast<ExternalFrameBufferList*>(user_priv);
 return fb_list->ReturnFrameBuffer(fb);
}
