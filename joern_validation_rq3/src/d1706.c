cmsBool ReadOffsetArray(cmsIOHANDLER* io,  _cmsDICarray* a, cmsUInt32Number Count, cmsUInt32Number Length, cmsUInt32Number BaseOffset)
{
    cmsUInt32Number i;

    for (i=0; i < Count; i++) {

        if (!ReadOneElem(io, &a -> Name, i, BaseOffset)) return FALSE;
        if (!ReadOneElem(io, &a -> Value, i, BaseOffset)) return FALSE;

        if (Length > 16) {

            if (!ReadOneElem(io, &a ->DisplayName, i, BaseOffset)) return FALSE;

        }

        if (Length > 24) {

            if (!ReadOneElem(io, & a -> DisplayValue, i, BaseOffset)) return FALSE;
        }
    }
    return TRUE;
}
