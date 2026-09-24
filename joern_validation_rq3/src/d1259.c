int LocateSample(cmsIT8* it8, const char* cSample)
{
    int i;
    const char *fld;
    TABLE* t = GetTable(it8);

    for (i=0; i < t->nSamples; i++) {

        fld = GetDataFormat(it8, i);
        if (fld != NULL) {
            if (cmsstrcasecmp(fld, cSample) == 0)
                return i;
        }
    }

    return -1;

}
