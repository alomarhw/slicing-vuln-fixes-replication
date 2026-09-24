static int vrend_decode_set_clip_state(struct vrend_decode_ctx *ctx, int length)
{
   struct pipe_clip_state clip;
   int i, j;

   if (length != VIRGL_SET_CLIP_STATE_SIZE)
      return EINVAL;

   for (i = 0; i < 8; i++)
      for (j = 0; j < 4; j++)
         clip.ucp[i][j] = uif(get_buf_entry(ctx, VIRGL_SET_CLIP_STATE_C0 + (i * 4) + j));
   vrend_set_clip_state(ctx->grctx, &clip);
   return 0;
}
