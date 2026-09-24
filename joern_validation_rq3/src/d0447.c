SProcXvQueryAdaptors(ClientPtr client)
{
    REQUEST(xvQueryAdaptorsReq);
    REQUEST_SIZE_MATCH(xvQueryAdaptorsReq);
    swaps(&stuff->length);
    swapl(&stuff->window);
    return XvProcVector[xv_QueryAdaptors] (client);
}
