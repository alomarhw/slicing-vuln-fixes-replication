static int vrend_decode_set_stencil_ref(struct vrend_decode_ctx *ctx, int length)
{
   if (length != VIRGL_SET_STENCIL_REF_SIZE)
      return EINVAL;

   struct pipe_stencil_ref ref;
   uint32_t val = get_buf_entry(ctx, VIRGL_SET_STENCIL_REF);

   ref.ref_value[0] = val & 0xff;
   ref.ref_value[1] = (val >> 8) & 0xff;
   vrend_set_stencil_ref(ctx->grctx, &ref);
   return 0;
}
