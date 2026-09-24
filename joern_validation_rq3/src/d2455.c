SProcRenderAddGlyphs (ClientPtr client)
{
    register int n;
    register int i;
    CARD32  *gids;
    void    *end;
    xGlyphInfo *gi;
    REQUEST(xRenderAddGlyphsReq);
    swaps(&stuff->length, n);
    swapl(&stuff->glyphset, n);
    swapl(&stuff->nglyphs, n);
    if (stuff->nglyphs & 0xe0000000)
	return BadLength;
    end = (CARD8 *) stuff + (client->req_len << 2);
    gids = (CARD32 *) (stuff + 1);
    gi = (xGlyphInfo *) (gids + stuff->nglyphs);
    if ((char *) end - (char *) (gids + stuff->nglyphs) < 0)
	return BadLength;
    if ((char *) end - (char *) (gi + stuff->nglyphs) < 0)
	return BadLength;
    for (i = 0; i < stuff->nglyphs; i++)
    {
	swapl (&gids[i], n);
	swaps (&gi[i].width, n);
	swaps (&gi[i].height, n);
	swaps (&gi[i].x, n);
	swaps (&gi[i].y, n);
	swaps (&gi[i].xOff, n);
	swaps (&gi[i].yOff, n);
    }
    return (*ProcRenderVector[stuff->renderReqType]) (client);
}
