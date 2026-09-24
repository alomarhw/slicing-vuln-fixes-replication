hfs_cat_get_record_offset(HFS_INFO * hfs, const hfs_btree_key_cat * needle)
{
    HFS_CAT_GET_RECORD_OFFSET_DATA offset_data;
    offset_data.off = 0;
    offset_data.targ_key = needle;
    if (hfs_cat_traverse(hfs, hfs_cat_get_record_offset_cb, &offset_data)) {
        return 0;
    }
    return offset_data.off;
}
