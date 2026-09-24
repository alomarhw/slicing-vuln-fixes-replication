UngrabServer(ClientPtr client)
{
    int i;

    grabState = GrabNone;
    ListenToAllClients();
    mark_client_ungrab();
    for (i = mskcnt; --i >= 0 && !grabWaiters[i];);
    if (i >= 0) {
        i <<= 5;
        while (!GETBIT(grabWaiters, i))
            i++;
        BITCLEAR(grabWaiters, i);
        AttendClient(clients[i]);
    }

    if (ServerGrabCallback) {
        ServerGrabInfoRec grabinfo;

        grabinfo.client = client;
        grabinfo.grabstate = SERVER_UNGRABBED;
        CallCallbacks(&ServerGrabCallback, (void *) &grabinfo);
    }
}
