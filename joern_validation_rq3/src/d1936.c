static const char *register_input_filter(cmd_parms *cmd, void *_cfg,
                                       const char* filter, const char *file, const char *function)
{
    const char *err = ap_check_cmd_context(cmd, NOT_IN_DIRECTORY|NOT_IN_FILES|
                                                NOT_IN_HTACCESS);
    if (err) {
        return err;
    }
    if (!function) function = "handle";
    return register_filter_function_hook(filter, cmd, _cfg, file,
                                             function, AP_LUA_FILTER_INPUT);
}
