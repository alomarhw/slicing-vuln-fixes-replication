void GLES2Implementation::SetGLErrorInvalidEnum(const char* function_name,
                                                GLenum value,
                                                const char* label) {
  SetGLError(
      GL_INVALID_ENUM, function_name,
      (std::string(label) + " was " + GLES2Util::GetStringEnum(value)).c_str());
}
