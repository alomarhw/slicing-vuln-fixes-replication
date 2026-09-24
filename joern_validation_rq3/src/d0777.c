move_copy_file_callback_new (GFile    *file,
                             gboolean  overwrite,
                             GdkPoint *position)
{
    MoveFileCopyFallback *fallback;

    fallback = g_new (MoveFileCopyFallback, 1);
    fallback->file = file;
    fallback->overwrite = overwrite;
    if (position)
    {
        fallback->has_position = TRUE;
        fallback->position = *position;
    }
    else
    {
        fallback->has_position = FALSE;
    }

    return fallback;
}
