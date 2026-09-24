CloseDownClient(ClientPtr client)
{
    Bool really_close_down = client->clientGone ||
        client->closeDownMode == DestroyAll;

    if (!client->clientGone) {
        /* ungrab server if grabbing client dies */
        if (grabState != GrabNone && grabClient == client) {
            UngrabServer(client);
        }
        BITCLEAR(grabWaiters, client->index);
        DeleteClientFromAnySelections(client);
        ReleaseActiveGrabs(client);
        DeleteClientFontStuff(client);
        if (!really_close_down) {
            /*  This frees resources that should never be retained
             *  no matter what the close down mode is.  Actually we
             *  could do this unconditionally, but it's probably
             *  better not to traverse all the client's resources
             *  twice (once here, once a few lines down in
             *  FreeClientResources) in the common case of
             *  really_close_down == TRUE.
             */
            FreeClientNeverRetainResources(client);
            client->clientState = ClientStateRetained;
            if (ClientStateCallback) {
                NewClientInfoRec clientinfo;

                clientinfo.client = client;
                clientinfo.prefix = (xConnSetupPrefix *) NULL;
                clientinfo.setup = (xConnSetup *) NULL;
                CallCallbacks((&ClientStateCallback), (void *) &clientinfo);
            }
        }
        client->clientGone = TRUE;      /* so events aren't sent to client */
        if (ClientIsAsleep(client))
            ClientSignal(client);
        ProcessWorkQueueZombies();
        CloseDownConnection(client);
        output_pending_clear(client);
        mark_client_not_ready(client);

        /* If the client made it to the Running stage, nClients has
         * been incremented on its behalf, so we need to decrement it
         * now.  If it hasn't gotten to Running, nClients has *not*
         * been incremented, so *don't* decrement it.
         */
        if (client->clientState != ClientStateInitial) {
            --nClients;
        }
    }

    if (really_close_down) {
        if (client->clientState == ClientStateRunning && nClients == 0)
            dispatchException |= dispatchExceptionAtReset;

        client->clientState = ClientStateGone;
        if (ClientStateCallback) {
            NewClientInfoRec clientinfo;

            clientinfo.client = client;
            clientinfo.prefix = (xConnSetupPrefix *) NULL;
            clientinfo.setup = (xConnSetup *) NULL;
            CallCallbacks((&ClientStateCallback), (void *) &clientinfo);
        }
        TouchListenerGone(client->clientAsMask);
        FreeClientResources(client);
        /* Disable client ID tracking. This must be done after
         * ClientStateCallback. */
        ReleaseClientIds(client);
#ifdef XSERVER_DTRACE
        XSERVER_CLIENT_DISCONNECT(client->index);
#endif
        if (client->index < nextFreeClientID)
            nextFreeClientID = client->index;
        clients[client->index] = NullClient;
        SmartLastClient = NullClient;
        dixFreeObjectWithPrivates(client, PRIVATE_CLIENT);

        while (!clients[currentMaxClients - 1])
            currentMaxClients--;
    }
}
