error::ContextLostReason GLES2DecoderImpl::GetContextLostReason() {
  switch (reset_status_) {
  case GL_NO_ERROR:
    return error::kUnknown;
  case GL_GUILTY_CONTEXT_RESET_ARB:
    return error::kGuilty;
  case GL_INNOCENT_CONTEXT_RESET_ARB:
    return error::kInnocent;
  case GL_UNKNOWN_CONTEXT_RESET_ARB:
    return error::kUnknown;
  }

  NOTREACHED();
  return error::kUnknown;
}
