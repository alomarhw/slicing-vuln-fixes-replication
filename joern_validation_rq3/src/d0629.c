void WebGL2RenderingContextBase::clearBufferiv(GLenum buffer,
                                               GLint drawbuffer,
                                               const Vector<GLint>& value,
                                               GLuint src_offset) {
  if (isContextLost() ||
      !ValidateClearBuffer("clearBufferiv", buffer, value.size(), src_offset))
    return;

  ScopedRGBEmulationColorMask emulation_color_mask(this, color_mask_,
                                                   drawing_buffer_.get());

  ContextGL()->ClearBufferiv(buffer, drawbuffer, value.data() + src_offset);
}
