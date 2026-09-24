error::Error GLES2DecoderPassthroughImpl::DoGetProgramResourceiv(
    GLuint program,
    GLenum program_interface,
    GLuint index,
    GLsizei prop_count,
    const GLenum* props,
    GLsizei bufsize,
    GLsizei* length,
    GLint* params) {
  api()->glGetProgramResourceivFn(GetProgramServiceID(program, resources_),
                                  program_interface, index, prop_count, props,
                                  bufsize, length, params);
  return error::kNoError;
}
