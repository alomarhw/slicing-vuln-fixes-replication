int CMS_uncompress(CMS_ContentInfo *cms, BIO *dcont, BIO *out,
                   unsigned int flags)
{
    BIO *cont;
    int r;
    if (OBJ_obj2nid(CMS_get0_type(cms)) != NID_id_smime_ct_compressedData) {
        CMSerr(CMS_F_CMS_UNCOMPRESS, CMS_R_TYPE_NOT_COMPRESSED_DATA);
        return 0;
    }

    if (!dcont && !check_content(cms))
        return 0;

    cont = CMS_dataInit(cms, dcont);
    if (!cont)
        return 0;
    r = cms_copy_content(out, cont, flags);
    do_free_upto(cont, dcont);
    return r;
}
