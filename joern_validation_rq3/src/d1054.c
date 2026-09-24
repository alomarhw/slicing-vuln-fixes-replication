ProcCirculateWindow(ClientPtr client)
{
    WindowPtr pWin;

    REQUEST(xCirculateWindowReq);
    int rc;

    REQUEST_SIZE_MATCH(xCirculateWindowReq);
    if ((stuff->direction != RaiseLowest) && (stuff->direction != LowerHighest)) {
        client->errorValue = stuff->direction;
        return BadValue;
    }
    rc = dixLookupWindow(&pWin, stuff->window, client, DixManageAccess);
    if (rc != Success)
        return rc;
    CirculateWindow(pWin, (int) stuff->direction, client);
    return Success;
}
