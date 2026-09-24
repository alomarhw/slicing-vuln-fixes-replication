delete_task_thread_func (GTask        *task,
                         gpointer      source_object,
                         gpointer      task_data,
                         GCancellable *cancellable)
{
    DeleteJob *job = task_data;
    GList *to_trash_files;
    GList *to_delete_files;
    GList *l;
    GFile *file;
    gboolean confirmed;
    CommonJob *common;
    gboolean must_confirm_delete_in_trash;
    gboolean must_confirm_delete;
    int files_skipped;

    common = (CommonJob *) job;

    nautilus_progress_info_start (job->common.progress);

    to_trash_files = NULL;
    to_delete_files = NULL;

    must_confirm_delete_in_trash = FALSE;
    must_confirm_delete = FALSE;
    files_skipped = 0;

    for (l = job->files; l != NULL; l = l->next)
    {
        file = l->data;

        if (job->try_trash &&
            g_file_has_uri_scheme (file, "trash"))
        {
            must_confirm_delete_in_trash = TRUE;
            to_delete_files = g_list_prepend (to_delete_files, file);
        }
        else if (can_delete_without_confirm (file))
        {
            to_delete_files = g_list_prepend (to_delete_files, file);
        }
        else
        {
            if (job->try_trash)
            {
                to_trash_files = g_list_prepend (to_trash_files, file);
            }
            else
            {
                must_confirm_delete = TRUE;
                to_delete_files = g_list_prepend (to_delete_files, file);
            }
        }
    }

    if (to_delete_files != NULL)
    {
        to_delete_files = g_list_reverse (to_delete_files);
        confirmed = TRUE;
        if (must_confirm_delete_in_trash)
        {
            confirmed = confirm_delete_from_trash (common, to_delete_files);
        }
        else if (must_confirm_delete)
        {
            confirmed = confirm_delete_directly (common, to_delete_files);
        }
        if (confirmed)
        {
            delete_files (common, to_delete_files, &files_skipped);
        }
        else
        {
            job->user_cancel = TRUE;
        }
    }

    if (to_trash_files != NULL)
    {
        to_trash_files = g_list_reverse (to_trash_files);

        trash_files (common, to_trash_files, &files_skipped);
    }

    g_list_free (to_trash_files);
    g_list_free (to_delete_files);

    if (files_skipped == g_list_length (job->files))
    {
        /* User has skipped all files, report user cancel */
        job->user_cancel = TRUE;
    }
}
