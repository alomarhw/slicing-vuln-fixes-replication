FilterRawEvents(const ClientPtr client, const GrabPtr grab, WindowPtr root)
{
    XIClientPtr client_xi_version;
    int cmp;

    /* device not grabbed -> don't filter */
    if (!grab)
        return FALSE;

    client_xi_version =
        dixLookupPrivate(&client->devPrivates, XIClientPrivateKey);

    cmp = version_compare(client_xi_version->major_version,
                          client_xi_version->minor_version, 2, 0);
    /* XI 2.0: if device is grabbed, skip
       XI 2.1: if device is grabbed by us, skip, we've already delivered */
    if (cmp == 0)
        return TRUE;

    return (grab->window != root) ? FALSE : SameClient(grab, client);
}
