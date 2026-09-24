error::Error GLES2DecoderPassthroughImpl::DoUniform3ui(GLint location,
                                                       GLuint x,
                                                       GLuint y,
                                                       GLuint z) {
  api()->glUniform3uiFn(location, x, y, z);
  return error::kNoError;
}
