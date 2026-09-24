cmsBool GrowMLUtable(cmsMLU* mlu)
{
    int AllocatedEntries;
    _cmsMLUentry *NewPtr;

    if (mlu == NULL) return FALSE;

    AllocatedEntries = mlu ->AllocatedEntries * 2;

    if (AllocatedEntries / 2 != mlu ->AllocatedEntries) return FALSE;

    NewPtr = (_cmsMLUentry*)_cmsRealloc(mlu ->ContextID, mlu ->Entries, AllocatedEntries*sizeof(_cmsMLUentry));
    if (NewPtr == NULL) return FALSE;

    mlu ->Entries          = NewPtr;
    mlu ->AllocatedEntries = AllocatedEntries;

    return TRUE;
}
