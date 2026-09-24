query_fs_type (GFile        *file,
               GCancellable *cancellable)
{
    GFileInfo *fsinfo;
    char *ret;

    ret = NULL;

    fsinfo = g_file_query_filesystem_info (file,
                                           G_FILE_ATTRIBUTE_FILESYSTEM_TYPE,
                                           cancellable,
                                           NULL);
    if (fsinfo != NULL)
    {
        ret = g_strdup (g_file_info_get_attribute_string (fsinfo, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE));
        g_object_unref (fsinfo);
    }

    if (ret == NULL)
    {
        /* ensure that we don't attempt to query
         * the FS type for each file in a given
         * directory, if it can't be queried. */
        ret = g_strdup ("");
    }

    return ret;
}
