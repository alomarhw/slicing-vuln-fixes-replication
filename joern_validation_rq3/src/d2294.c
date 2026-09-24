SProcPseudoramiXGetState(ClientPtr client)
{
    REQUEST(xPanoramiXGetStateReq);

    TRACE;

    swaps(&stuff->length);
    REQUEST_SIZE_MATCH(xPanoramiXGetStateReq);
    return ProcPseudoramiXGetState(client);
}
