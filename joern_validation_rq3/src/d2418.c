extract_job_on_completed (AutoarExtractor *extractor,
                          gpointer         user_data)
{
    ExtractJob *extract_job = user_data;
    GFile *output_file;

    output_file = G_FILE (extract_job->output_files->data);

    nautilus_file_changes_queue_file_added (output_file);
}
