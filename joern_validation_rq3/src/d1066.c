void WebGL2RenderingContextBase::uniform3iv(
    const WebGLUniformLocation* location,
    const FlexibleInt32ArrayView& v,
    GLuint src_offset,
    GLuint src_length) {
  if (isContextLost() ||
      !ValidateUniformParameters<WTF::Int32Array>("uniform3iv", location, v, 3,
                                                  src_offset, src_length))
    return;

  ContextGL()->Uniform3iv(
      location->Location(),
      (src_length ? src_length : (v.length() - src_offset)) / 3,
      v.DataMaybeOnStack() + src_offset);
}
