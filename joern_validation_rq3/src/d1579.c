ProcRenderTriangles (ClientPtr client)
{
    int		rc, ntris;
    PicturePtr	pSrc, pDst;
    PictFormatPtr   pFormat;
    REQUEST(xRenderTrianglesReq);

    REQUEST_AT_LEAST_SIZE(xRenderTrianglesReq);
    if (!PictOpValid (stuff->op))
    {
	client->errorValue = stuff->op;
	return BadValue;
    }
    VERIFY_PICTURE (pSrc, stuff->src, client, DixReadAccess);
    VERIFY_PICTURE (pDst, stuff->dst, client, DixWriteAccess);
    if (!pDst->pDrawable)
        return BadDrawable;
    if (pSrc->pDrawable && pSrc->pDrawable->pScreen != pDst->pDrawable->pScreen)
	return BadMatch;
    if (stuff->maskFormat)
    {
	rc = dixLookupResourceByType((pointer *)&pFormat, stuff->maskFormat,
				     PictFormatType, client, DixReadAccess);
	if (rc != Success)
	    return rc;
    }
    else
	pFormat = 0;
    ntris = (client->req_len << 2) - sizeof (xRenderTrianglesReq);
    if (ntris % sizeof (xTriangle))
	return BadLength;
    ntris /= sizeof (xTriangle);
    if (ntris)
	CompositeTriangles (stuff->op, pSrc, pDst, pFormat,
			    stuff->xSrc, stuff->ySrc,
			    ntris, (xTriangle *) &stuff[1]);
    return Success;
}
