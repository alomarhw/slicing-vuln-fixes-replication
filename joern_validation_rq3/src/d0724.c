static int rgbinitialproc(i_ctx_t *i_ctx_p, ref *space)
{
    gs_client_color cc;

    cc.pattern = 0x00;
    cc.paint.values[0] = 0;
    cc.paint.values[1] = 0;
    cc.paint.values[2] = 0;
    return gs_setcolor(igs, &cc);
}
