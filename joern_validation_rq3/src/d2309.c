static int vrend_decode_bind_shader(struct vrend_decode_ctx *ctx, int length)
{
   uint32_t handle, type;
   if (length != VIRGL_BIND_SHADER_SIZE)
      return EINVAL;

   handle = get_buf_entry(ctx, VIRGL_BIND_SHADER_HANDLE);
   type = get_buf_entry(ctx, VIRGL_BIND_SHADER_TYPE);

   vrend_bind_shader(ctx->grctx, handle, type);
   return 0;
}
