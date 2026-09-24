static int vrend_decode_set_vertex_buffers(struct vrend_decode_ctx *ctx, uint16_t length)
{
   int num_vbo;
   int i;

   /* must be a multiple of 3 */
   if (length && (length % 3))
      return EINVAL;

   num_vbo = (length / 3);
   if (num_vbo > PIPE_MAX_ATTRIBS)
      return EINVAL;

   for (i = 0; i < num_vbo; i++) {
      vrend_set_single_vbo(ctx->grctx, i,
                           get_buf_entry(ctx, VIRGL_SET_VERTEX_BUFFER_STRIDE(i)),
                           get_buf_entry(ctx, VIRGL_SET_VERTEX_BUFFER_OFFSET(i)),
                           get_buf_entry(ctx, VIRGL_SET_VERTEX_BUFFER_HANDLE(i)));
   }
   vrend_set_num_vbo(ctx->grctx, num_vbo);
   return 0;
}
