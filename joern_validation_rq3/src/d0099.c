void ssh_connshare_log(Ssh ssh, int event, const char *logtext,
                       const char *ds_err, const char *us_err)
{
    if (event == SHARE_NONE) {
        /* In this case, 'logtext' is an error message indicating a
         * reason why connection sharing couldn't be set up _at all_.
         * Failing that, ds_err and us_err indicate why we couldn't be
         * a downstream and an upstream respectively. */
        if (logtext) {
            logeventf(ssh, "Could not set up connection sharing: %s", logtext);
        } else {
            if (ds_err)
                logeventf(ssh, "Could not set up connection sharing"
                          " as downstream: %s", ds_err);
            if (us_err)
                logeventf(ssh, "Could not set up connection sharing"
                          " as upstream: %s", us_err);
        }
    } else if (event == SHARE_DOWNSTREAM) {
        /* In this case, 'logtext' is a local endpoint address */
        logeventf(ssh, "Using existing shared connection at %s", logtext);
        /* Also we should mention this in the console window to avoid
         * confusing users as to why this window doesn't behave the
         * usual way. */
        if ((flags & FLAG_VERBOSE) || (flags & FLAG_INTERACTIVE)) {
            c_write_str(ssh,"Reusing a shared connection to this server.\r\n");
        }
    } else if (event == SHARE_UPSTREAM) {
        /* In this case, 'logtext' is a local endpoint address too */
        logeventf(ssh, "Sharing this connection at %s", logtext);
    }
}
