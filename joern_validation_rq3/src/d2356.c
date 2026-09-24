XineramaCopyGC(GCPtr pGCSrc, unsigned long mask, GCPtr pGCDst)
{
    PanoramiXGCPtr pSrcPriv = (PanoramiXGCPtr)
        dixLookupPrivate(&pGCSrc->devPrivates, PanoramiXGCKey);

    Xinerama_GC_FUNC_PROLOGUE(pGCDst);

    if (mask & GCTileStipXOrigin)
        pGCPriv->patOrg.x = pSrcPriv->patOrg.x;
    if (mask & GCTileStipYOrigin)
        pGCPriv->patOrg.y = pSrcPriv->patOrg.y;
    if (mask & GCClipXOrigin)
        pGCPriv->clipOrg.x = pSrcPriv->clipOrg.x;
    if (mask & GCClipYOrigin)
        pGCPriv->clipOrg.y = pSrcPriv->clipOrg.y;

    (*pGCDst->funcs->CopyGC) (pGCSrc, mask, pGCDst);
    Xinerama_GC_FUNC_EPILOGUE(pGCDst);
}
