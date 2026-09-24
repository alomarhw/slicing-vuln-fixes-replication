verify_destination (CommonJob  *job,
                    GFile      *dest,
                    char      **dest_fs_id,
                    goffset     required_size)
{
    GFileInfo *info, *fsinfo;
    GError *error;
    guint64 free_size;
    guint64 size_difference;
    char *primary, *secondary, *details;
    int response;
    GFileType file_type;
    gboolean dest_is_symlink = FALSE;

    if (dest_fs_id)
    {
        *dest_fs_id = NULL;
    }

retry:

    error = NULL;
    info = g_file_query_info (dest,
                              G_FILE_ATTRIBUTE_STANDARD_TYPE ","
                              G_FILE_ATTRIBUTE_ID_FILESYSTEM,
                              dest_is_symlink ? G_FILE_QUERY_INFO_NONE : G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                              job->cancellable,
                              &error);

    if (info == NULL)
    {
        if (IS_IO_ERROR (error, CANCELLED))
        {
            g_error_free (error);
            return;
        }

        primary = f (_("Error while copying to “%B”."), dest);
        details = NULL;

        if (IS_IO_ERROR (error, PERMISSION_DENIED))
        {
            secondary = f (_("You do not have permissions to access the destination folder."));
        }
        else
        {
            secondary = f (_("There was an error getting information about the destination."));
            details = error->message;
        }

        response = run_error (job,
                              primary,
                              secondary,
                              details,
                              FALSE,
                              CANCEL, RETRY,
                              NULL);

        g_error_free (error);

        if (response == 0 || response == GTK_RESPONSE_DELETE_EVENT)
        {
            abort_job (job);
        }
        else if (response == 1)
        {
            goto retry;
        }
        else
        {
            g_assert_not_reached ();
        }

        return;
    }

    file_type = g_file_info_get_file_type (info);
    if (!dest_is_symlink && file_type == G_FILE_TYPE_SYMBOLIC_LINK)
    {
        /* Record that destination is a symlink and do real stat() once again */
        dest_is_symlink = TRUE;
        g_object_unref (info);
        goto retry;
    }

    if (dest_fs_id)
    {
        *dest_fs_id =
            g_strdup (g_file_info_get_attribute_string (info,
                                                        G_FILE_ATTRIBUTE_ID_FILESYSTEM));
    }

    g_object_unref (info);

    if (file_type != G_FILE_TYPE_DIRECTORY)
    {
        primary = f (_("Error while copying to “%B”."), dest);
        secondary = f (_("The destination is not a folder."));

        run_error (job,
                   primary,
                   secondary,
                   NULL,
                   FALSE,
                   CANCEL,
                   NULL);

        abort_job (job);
        return;
    }

    if (dest_is_symlink)
    {
        /* We can't reliably statfs() destination if it's a symlink, thus not doing any further checks. */
        return;
    }

    fsinfo = g_file_query_filesystem_info (dest,
                                           G_FILE_ATTRIBUTE_FILESYSTEM_FREE ","
                                           G_FILE_ATTRIBUTE_FILESYSTEM_READONLY,
                                           job->cancellable,
                                           NULL);
    if (fsinfo == NULL)
    {
        /* All sorts of things can go wrong getting the fs info (like not supported)
         * only check these things if the fs returns them
         */
        return;
    }

    if (required_size > 0 &&
        g_file_info_has_attribute (fsinfo, G_FILE_ATTRIBUTE_FILESYSTEM_FREE))
    {
        free_size = g_file_info_get_attribute_uint64 (fsinfo,
                                                      G_FILE_ATTRIBUTE_FILESYSTEM_FREE);

        if (free_size < required_size)
        {
            size_difference = required_size - free_size;
            primary = f (_("Error while copying to “%B”."), dest);
            secondary = f (_("There is not enough space on the destination. Try to remove files to make space."));

            details = f (_("%S more space is required to copy to the destination."), size_difference);

            response = run_warning (job,
                                    primary,
                                    secondary,
                                    details,
                                    FALSE,
                                    CANCEL,
                                    COPY_FORCE,
                                    RETRY,
                                    NULL);

            if (response == 0 || response == GTK_RESPONSE_DELETE_EVENT)
            {
                abort_job (job);
            }
            else if (response == 2)
            {
                goto retry;
            }
            else if (response == 1)
            {
                /* We are forced to copy - just fall through ... */
            }
            else
            {
                g_assert_not_reached ();
            }
        }
    }

    if (!job_aborted (job) &&
        g_file_info_get_attribute_boolean (fsinfo,
                                           G_FILE_ATTRIBUTE_FILESYSTEM_READONLY))
    {
        primary = f (_("Error while copying to “%B”."), dest);
        secondary = f (_("The destination is read-only."));

        run_error (job,
                   primary,
                   secondary,
                   NULL,
                   FALSE,
                   CANCEL,
                   NULL);

        g_error_free (error);

        abort_job (job);
    }

    g_object_unref (fsinfo);
}
