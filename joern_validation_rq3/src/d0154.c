SProcXvDispatch(ClientPtr client)
{
    REQUEST(xReq);

    UpdateCurrentTime();

    if (stuff->data >= xvNumRequests) {
        return BadRequest;
    }

    return SXvProcVector[stuff->data] (client);
}
