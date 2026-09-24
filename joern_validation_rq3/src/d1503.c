error::Error GLES2DecoderImpl::HandleVertexAttribDivisorANGLE(
    uint32 immediate_data_size, const gles2::VertexAttribDivisorANGLE& c) {
  if (!feature_info_->feature_flags().angle_instanced_arrays) {
    SetGLError(GL_INVALID_OPERATION,
               "glVertexAttribDivisorANGLE: function not available");
  }
  GLuint index = c.index;
  GLuint divisor = c.divisor;
  if (index >= group_->max_vertex_attribs()) {
    SetGLError(GL_INVALID_VALUE,
               "glVertexAttribDivisorANGLE: index out of range");
    return error::kNoError;
  }

  vertex_attrib_manager_->SetDivisor(
      index,
      divisor);
  glVertexAttribDivisorANGLE(index, divisor);
  return error::kNoError;
}
