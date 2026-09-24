cmsUInt32Number CMSEXPORT cmsGetSupportedIntents(cmsUInt32Number nMax, cmsUInt32Number* Codes, char** Descriptions)
{
    cmsIntentsList* pt;
    cmsUInt32Number nIntents;

    for (nIntents=0, pt = Intents; pt != NULL; pt = pt -> Next)
    {
        if (nIntents < nMax) {
            if (Codes != NULL)
                Codes[nIntents] = pt ->Intent;

            if (Descriptions != NULL)
                Descriptions[nIntents] = pt ->Description;
        }

        nIntents++;
    }

    return nIntents;
}
