gs_currentsrcgtagicc(const gs_gstate * pgs, gs_param_string * pval)
{
    if (pgs->icc_manager->srcgtag_profile == NULL) {
        pval->data = NULL;
        pval->size = 0;
        pval->persistent = true;
    } else {
        pval->data = (byte *)pgs->icc_manager->srcgtag_profile->name;
        pval->size = strlen((const char *)pval->data);
        pval->persistent = false;
    }
}
