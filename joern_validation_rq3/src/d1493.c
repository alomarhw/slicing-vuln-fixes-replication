bool GLES2DecoderImpl::IsDrawValid(
    GLuint max_vertex_accessed, GLsizei primcount) {
  if (!current_program_) {
    return false;
  }

  bool divisor0 = false;
  const VertexAttribManager::VertexAttribInfoList& infos =
      vertex_attrib_manager_->GetEnabledVertexAttribInfos();
  for (VertexAttribManager::VertexAttribInfoList::const_iterator it =
       infos.begin(); it != infos.end(); ++it) {
    const VertexAttribManager::VertexAttribInfo* info = *it;
    const ProgramManager::ProgramInfo::VertexAttribInfo* attrib_info =
        current_program_->GetAttribInfoByLocation(info->index());
    if (attrib_info) {
      divisor0 |= (info->divisor() == 0);
      GLuint count = info->MaxVertexAccessed(primcount, max_vertex_accessed);
      if (!info->CanAccess(count)) {
        SetGLError(GL_INVALID_OPERATION,
                   "glDrawXXX: attempt to access out of range vertices");
        return false;
      }
    } else {
      if (!info->buffer()) {
        SetGLError(
            GL_INVALID_OPERATION,
            "glDrawXXX: attempt to render with no buffer attached to enabled "
            "attrib");
        return false;
      }
    }
  }

  if (primcount && !divisor0) {
    SetGLError(
        GL_INVALID_OPERATION,
        "glDrawXXX: attempt instanced render with all attributes having "
        "non-zero divisors");
    return false;
  }

  return true;
}
