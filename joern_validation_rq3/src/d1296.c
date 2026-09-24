XmpPtr xmp_new_empty()
{
    RESET_ERROR;
    SXMPMeta *txmp = new SXMPMeta;
    return (XmpPtr)txmp;
}
