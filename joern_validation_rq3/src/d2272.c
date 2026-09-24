validGlxFBConfig(ClientPtr client, __GLXscreen *pGlxScreen, XID id,
		 __GLXconfig **config, int *err)
{
    __GLXconfig *m;

    for (m = pGlxScreen->fbconfigs; m != NULL; m = m->next)
	if (m->fbconfigID == id) {
	    *config = m;
	    return TRUE;
	}

    client->errorValue = id;
    *err = __glXError(GLXBadFBConfig);

    return FALSE;
}
