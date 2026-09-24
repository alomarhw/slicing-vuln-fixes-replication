static gboolean _can_be_exec(MenuCacheApp *app)
{
    char *path;

    if (app->try_exec == NULL)
        return TRUE;
    path = g_find_program_in_path(app->try_exec);
    g_free(path);
    return (path != NULL);
}
