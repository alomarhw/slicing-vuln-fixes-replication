ScopedResolvedFrameBufferBinder::~ScopedResolvedFrameBufferBinder() {
  if (!resolve_and_bind_)
    return;

  ScopedGLErrorSuppressor suppressor(decoder_);
  decoder_->RestoreCurrentFramebufferBindings();
  if (decoder_->enable_scissor_test_) {
    glEnable(GL_SCISSOR_TEST);
  }
}
