    DrawingBufferClientRestorePixelUnpackBufferBinding() {
  if (!ContextGL())
    return;
  ContextGL()->BindBuffer(GL_PIXEL_UNPACK_BUFFER,
                          ObjectOrZero(bound_pixel_unpack_buffer_.Get()));
}
