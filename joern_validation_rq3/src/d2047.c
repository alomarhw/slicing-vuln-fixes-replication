void GLES2DecoderImpl::DoUniformMatrix4fv(
    GLint fake_location, GLsizei count, GLboolean transpose,
    const GLfloat* value) {
  GLenum type = 0;
  GLint real_location = -1;
  if (!PrepForSetUniformByLocation(fake_location,
                                   "glUniformMatrix4fv",
                                   Program::kUniformMatrix4f,
                                   &real_location,
                                   &type,
                                   &count)) {
    return;
  }
  glUniformMatrix4fv(real_location, count, transpose, value);
}
