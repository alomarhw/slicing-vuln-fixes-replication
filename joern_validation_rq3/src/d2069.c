bool VaapiWrapper::GetDerivedVaImage(VASurfaceID va_surface_id,
                                     VAImage* image,
                                     void** mem) {
  base::AutoLock auto_lock(*va_lock_);

  VAStatus va_res = vaSyncSurface(va_display_, va_surface_id);
  VA_SUCCESS_OR_RETURN(va_res, "Failed syncing surface", false);

  va_res = vaDeriveImage(va_display_, va_surface_id, image);
  VA_LOG_ON_ERROR(va_res, "vaDeriveImage failed");
  if (va_res != VA_STATUS_SUCCESS)
    return false;

  va_res = vaMapBuffer(va_display_, image->buf, mem);
  VA_LOG_ON_ERROR(va_res, "vaMapBuffer failed");
  if (va_res == VA_STATUS_SUCCESS)
    return true;

  va_res = vaDestroyImage(va_display_, image->image_id);
  VA_LOG_ON_ERROR(va_res, "vaDestroyImage failed");

  return false;
}
