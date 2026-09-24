error::Error GLES2DecoderImpl::HandleDrawElements(uint32 immediate_data_size,
                                                  const void* cmd_data) {
  const gles2::cmds::DrawElements& c =
      *static_cast<const gles2::cmds::DrawElements*>(cmd_data);
  return DoDrawElements("glDrawElements",
                        false,
                        static_cast<GLenum>(c.mode),
                        static_cast<GLsizei>(c.count),
                        static_cast<GLenum>(c.type),
                        static_cast<int32>(c.index_offset),
                        1);
}
