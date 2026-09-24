void WebGLRenderingContextBase::validateProgram(WebGLProgram* program) {
  if (isContextLost() || !ValidateWebGLObject("validateProgram", program))
    return;
  ContextGL()->ValidateProgram(ObjectOrZero(program));
}
