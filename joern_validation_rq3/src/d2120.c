bool BackTexture::AllocateStorage(
    const gfx::Size& size, GLenum format, bool zero) {
  DCHECK_NE(id(), 0u);
  ScopedGLErrorSuppressor suppressor("BackTexture::AllocateStorage",
                                     decoder_->state_.GetErrorState());
  ScopedTextureBinder binder(&decoder_->state_, id(), Target());
  uint32_t image_size = 0;
  GLES2Util::ComputeImageDataSizes(size.width(), size.height(), 1, format,
                                   GL_UNSIGNED_BYTE, 8, &image_size, nullptr,
                                   nullptr);

  bool success = false;
  size_ = size;
  if (decoder_->should_use_native_gmb_for_backbuffer_) {
    DestroyNativeGpuMemoryBuffer(true);
    success = AllocateNativeGpuMemoryBuffer(size, format, zero);
  } else {
    {
      std::unique_ptr<char[]> zero_data;
      if (zero) {
        zero_data.reset(new char[image_size]);
        memset(zero_data.get(), 0, image_size);
      }

      api()->glTexImage2DFn(Target(),
                            0,  // mip level
                            format, size.width(), size.height(),
                            0,  // border
                            format, GL_UNSIGNED_BYTE, zero_data.get());
    }

    decoder_->texture_manager()->SetLevelInfo(
        texture_ref_.get(), Target(),
        0,  // level
        GL_RGBA, size.width(), size.height(),
        1,  // depth
        0,  // border
        GL_RGBA, GL_UNSIGNED_BYTE, gfx::Rect(size));
    success = api()->glGetErrorFn() == GL_NO_ERROR;
  }

  if (success) {
    memory_tracker_.TrackMemFree(bytes_allocated_);
    bytes_allocated_ = image_size;
    memory_tracker_.TrackMemAlloc(bytes_allocated_);
  }
  return success;
}
