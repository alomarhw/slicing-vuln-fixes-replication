deep_count_next_dir (DeepCountState *state)
{
    GFile *location;
    NautilusFile *file;
    NautilusDirectory *directory;
    gboolean done;

    directory = state->directory;

    g_object_unref (state->deep_count_location);
    state->deep_count_location = NULL;

    done = FALSE;
    file = directory->details->deep_count_file;

    if (state->deep_count_subdirectories != NULL)
    {
        /* Work on a new directory. */
        location = state->deep_count_subdirectories->data;
        state->deep_count_subdirectories = g_list_remove
                                               (state->deep_count_subdirectories, location);
        deep_count_load (state, location);
        g_object_unref (location);
    }
    else
    {
        file->details->deep_counts_status = NAUTILUS_REQUEST_DONE;
        directory->details->deep_count_file = NULL;
        directory->details->deep_count_in_progress = NULL;
        deep_count_state_free (state);
        done = TRUE;
    }

    nautilus_file_updated_deep_count_in_progress (file);

    if (done)
    {
        nautilus_file_changed (file);
        async_job_end (directory, "deep count");
        nautilus_directory_async_state_changed (directory);
    }
}
