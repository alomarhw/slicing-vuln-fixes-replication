AppLayerProtoDetectThreadCtx *AppLayerProtoDetectGetCtxThread(void)
{
    SCEnter();

    AppLayerProtoDetectThreadCtx *alpd_tctx = NULL;
    MpmCtx *mpm_ctx;
    MpmThreadCtx *mpm_tctx;
    int i, j;
    PatIntId max_pat_id = 0;

    for (i = 0; i < FLOW_PROTO_DEFAULT; i++) {
        for (j = 0; j < 2; j++) {
            if (max_pat_id == 0) {
                max_pat_id = alpd_ctx.ctx_ipp[i].ctx_pm[j].max_pat_id;

            } else if (alpd_ctx.ctx_ipp[i].ctx_pm[j].max_pat_id &&
                    max_pat_id < alpd_ctx.ctx_ipp[i].ctx_pm[j].max_pat_id)
            {
                max_pat_id = alpd_ctx.ctx_ipp[i].ctx_pm[j].max_pat_id;
            }
        }
    }

    alpd_tctx = SCMalloc(sizeof(*alpd_tctx));
    if (alpd_tctx == NULL)
        goto error;
    memset(alpd_tctx, 0, sizeof(*alpd_tctx));

    /* Get the max pat id for all the mpm ctxs. */
    if (PmqSetup(&alpd_tctx->pmq) < 0)
        goto error;

    for (i = 0; i < FLOW_PROTO_DEFAULT; i++) {
        for (j = 0; j < 2; j++) {
            mpm_ctx = &alpd_ctx.ctx_ipp[i].ctx_pm[j].mpm_ctx;
            mpm_tctx = &alpd_tctx->mpm_tctx[i][j];
            mpm_table[mpm_ctx->mpm_type].InitThreadCtx(mpm_ctx, mpm_tctx);
        }
    }

    alpd_tctx->spm_thread_ctx = SpmMakeThreadCtx(alpd_ctx.spm_global_thread_ctx);
    if (alpd_tctx->spm_thread_ctx == NULL) {
        goto error;
    }

    goto end;
 error:
    if (alpd_tctx != NULL)
        AppLayerProtoDetectDestroyCtxThread(alpd_tctx);
    alpd_tctx = NULL;
 end:
    SCReturnPtr(alpd_tctx, "AppLayerProtoDetectThreadCtx");
}
