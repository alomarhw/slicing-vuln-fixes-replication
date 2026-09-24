error::Error GLES2DecoderImpl::HandleResizeCHROMIUM(
    uint32 immediate_data_size, const cmds::ResizeCHROMIUM& c) {
  if (!offscreen_target_frame_buffer_.get() && surface_->DeferDraws())
    return error::kDeferCommandUntilLater;

  GLuint width = static_cast<GLuint>(c.width);
  GLuint height = static_cast<GLuint>(c.height);
  GLfloat scale_factor = c.scale_factor;
  TRACE_EVENT2("gpu", "glResizeChromium", "width", width, "height", height);

  width = std::max(1U, width);
  height = std::max(1U, height);

#if defined(OS_POSIX) && !defined(OS_MACOSX) && \
    !defined(UI_COMPOSITOR_IMAGE_TRANSPORT)
  glFinish();
#endif
  bool is_offscreen = !!offscreen_target_frame_buffer_.get();
  if (is_offscreen) {
    if (!ResizeOffscreenFrameBuffer(gfx::Size(width, height))) {
      LOG(ERROR) << "GLES2DecoderImpl: Context lost because "
                 << "ResizeOffscreenFrameBuffer failed.";
      return error::kLostContext;
    }
  }

  if (!resize_callback_.is_null()) {
    resize_callback_.Run(gfx::Size(width, height), scale_factor);
    DCHECK(context_->IsCurrent(surface_.get()));
    if (!context_->IsCurrent(surface_.get())) {
      LOG(ERROR) << "GLES2DecoderImpl: Context lost because context no longer "
                 << "current after resize callback.";
      return error::kLostContext;
    }
  }

  return error::kNoError;
}
