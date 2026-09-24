void *Type_Measurement_Read(struct _cms_typehandler_struct* self, cmsIOHANDLER* io, cmsUInt32Number* nItems, cmsUInt32Number SizeOfTag)
{
    cmsICCMeasurementConditions mc;

	
    memset(&mc, 0, sizeof(mc));
	
    if (!_cmsReadUInt32Number(io, &mc.Observer)) return NULL;
    if (!_cmsReadXYZNumber(io,    &mc.Backing)) return NULL;
    if (!_cmsReadUInt32Number(io, &mc.Geometry)) return NULL;
    if (!_cmsRead15Fixed16Number(io, &mc.Flare)) return NULL;
    if (!_cmsReadUInt32Number(io, &mc.IlluminantType)) return NULL;

    *nItems = 1;
    return _cmsDupMem(self ->ContextID, &mc, sizeof(cmsICCMeasurementConditions));

    cmsUNUSED_PARAMETER(SizeOfTag);
}
